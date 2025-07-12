#include <algorithm>

#include "DefaultAssetSystem.hpp"
#include "api/fx/IAssetService.hpp"
#include "api/fx/IEventQueue.hpp"
#include "as/Model.hpp"
#include "bk/ThreadName.hpp"
#include "buffers/BufferSystem.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "img/ImageManager.hpp"
#include "img/TextureArena.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "resources/ByteConverters.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "ModelProcessor.hpp"

namespace tr {

DefaultAssetSystem::DefaultAssetSystem(
    std::shared_ptr<IEventQueue> newEventQueue,
    std::shared_ptr<IAssetService> newAssetService,
    std::shared_ptr<BufferSystem> newBufferSystem,
    std::shared_ptr<GeometryBufferPack> newGeometryBufferPack,
    std::shared_ptr<TransferSystem> newTransferSystem,
    std::shared_ptr<GeometryAllocator> newGeometryAllocator,
    std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
    std::shared_ptr<TextureHandleMapper> newTextureHandleMapper,
    std::shared_ptr<ImageManager> newImageManager,
    std::shared_ptr<TextureArena> newTextureArena)
    : eventQueue{std::move(newEventQueue)},
      assetService{std::move(newAssetService)},
      bufferSystem{std::move(newBufferSystem)},
      geometryBufferPack{std::move(newGeometryBufferPack)},
      transferSystem{std::move(newTransferSystem)},
      geometryAllocator{std::move(newGeometryAllocator)},
      geometryHandleMapper{std::move(newGeometryHandleMapper)},
      textureHandleMapper{std::move(newTextureHandleMapper)},
      imageManager{std::move(newImageManager)},
      textureArena{std::move(newTextureArena)} {
  Log.trace("Constructing DefaultAssetSystem");
}

DefaultAssetSystem::~DefaultAssetSystem() {
  Log.trace("Destroying DefaultAssetSystem");
}

auto DefaultAssetSystem::run() -> void {
  ZoneScopedN("DefaultAssetSystem::run");
  Log.trace("DefaultAssetSystem::run()");

  thread = std::jthread([&](std::stop_token token) mutable {
    setCurrentThreadName("Assets");
    Log.trace("Started AssetSystemThread");
    // Create all subscriptions on the thread
    eventQueue->subscribe<BeginResourceBatch>(
        [this](const auto& batch) { eventBatches[batch->batchId] = std::vector<RequestVariant>{}; },
        "test_group");

    eventQueue->subscribe<StaticModelRequest>(
        [this](const auto& smRequest) { eventBatches[smRequest->batchId].push_back(smRequest); },
        "test_group");
    eventQueue->subscribe<StaticMeshRequest>(
        [this](const auto& smRequest) { eventBatches[smRequest->batchId].push_back(smRequest); },
        "test_group");

    eventQueue->subscribe<DynamicModelRequest>(
        [this](const auto& dmRequest) { eventBatches[dmRequest->batchId].push_back(dmRequest); },
        "test_group");

    eventQueue->subscribe<EndResourceBatch>(
        [this](const std::shared_ptr<EndResourceBatch>& batch) {
          processesBatchedResources(batch->batchId);
        },
        "test_group");

    while (!token.stop_requested()) {
      eventQueue->dispatchPending();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    Log.trace("AssetSystem thread shutting down");
  });
}

auto DefaultAssetSystem::requestStop() -> void {
  thread.request_stop();
}

auto DefaultAssetSystem::processesBatchedResources(uint64_t batchId) -> void {
  ZoneScoped;
  auto [uploadPlan, imageUploadPlan] = collectUploads(batchId);
  transferSystem->upload(uploadPlan, imageUploadPlan);
  finalizeResponses(uploadPlan, imageUploadPlan);
}

auto DefaultAssetSystem::collectUploads(uint64_t batchId)
    -> std::tuple<UploadPlan, ImageUploadPlan> {
  auto uploadPlan = UploadPlan{.stagingBuffer = transferSystem->getTransferContext().stagingBuffer};
  auto imageUploadPlan =
      ImageUploadPlan{.stagingBuffer = transferSystem->getTransferContext().imageStagingBuffer};

  struct RequestEventVisitor {
    UploadPlan& uploadPlan;
    ImageUploadPlan& imageUploadPlan;
    DefaultAssetSystem* assetSystem;

    void operator()(const std::shared_ptr<StaticModelRequest>& req) const {
      assetSystem->processStaticModelRequest(req, uploadPlan, imageUploadPlan);
    }

    void operator()(const std::shared_ptr<StaticMeshRequest>& req) const {
      assetSystem->processStaticMeshRequest(req, uploadPlan);
    }

    void operator()(const std::shared_ptr<DynamicModelRequest>& req) const {
      Log.trace("Handling Dynamic Model Request ID: {}", req->requestId);
    }
  };

  auto visitor = RequestEventVisitor{.uploadPlan = uploadPlan,
                                     .imageUploadPlan = imageUploadPlan,
                                     .assetSystem = this};

  for (auto& eventVariant : eventBatches[batchId]) {
    std::visit(visitor, eventVariant);
  }

  return {uploadPlan, imageUploadPlan};
}

auto DefaultAssetSystem::finalizeResponses(UploadPlan& uploadPlan, ImageUploadPlan& imageUploadPlan)
    -> void {
  // Post process image uploads, creating and adding texture handles to inFlightUploads
  for (const auto& [requestId, uploads] : imageUploadPlan.uploadsByRequest) {
    for (const auto& upload : uploads) {
      // Create a Texture (image+sampler)
      const auto& image = imageManager->getImage(upload.dstImage);
      const auto& sampler = imageManager->getSampler(imageManager->getDefaultSampler());
      auto handle = textureArena->insert(image.getImageView(), sampler);
      auto textureHandle = textureHandleMapper->toPublic(handle);
      // Set the texture in the *Uploaded event
      auto& inFlight = inFlightUploads.at(requestId);
      std::visit(ImageUploadPlan::ResponseEventVisitor{textureHandle}, inFlight.responseEvent);
      inFlight.remainingComponents--;
      if (inFlight.remainingComponents == 0) {
        std::visit(EmitEventVisitor{eventQueue}, inFlight.responseEvent);
        inFlightUploads.erase(requestId);
      }
    }
  }

  // Update inFlight uploads with geometry handles
  for (const auto& [requestId, bufferUpload] : uploadPlan.uploadsByRequest) {
    auto& inFlight = inFlightUploads.at(requestId);
    auto geometryHandle = uploadPlan.geometryDataByRequest.at(requestId);
    std::visit(UploadPlan::ResponseEventVisitor{geometryHandle}, inFlight.responseEvent);
    // Find some way to tighten this up a bit.
    inFlight.remainingComponents--;
    if (inFlight.remainingComponents == 0) {
      std::visit(EmitEventVisitor{eventQueue}, inFlight.responseEvent);
      inFlightUploads.erase(requestId);
    }
  }
}

auto DefaultAssetSystem::processStaticModelRequest(
    const std::shared_ptr<StaticModelRequest>& smRequest,
    UploadPlan& uploadPlan,
    ImageUploadPlan& imageUploadPlan) -> void {
  ModelProcessor::handle(smRequest,
                         uploadPlan,
                         imageUploadPlan,
                         inFlightUploads,
                         AssetSystems{
                             .assetService = assetService,
                             .geometryAllocator = geometryAllocator,
                             .transferSystem = transferSystem,
                             .geometryHandleMapper = geometryHandleMapper,
                             .imageManager = imageManager,
                         });
}

auto DefaultAssetSystem::processStaticMeshRequest(
    const std::shared_ptr<StaticMeshRequest>& smRequest,
    UploadPlan& uploadPlan) -> void {
  ZoneScoped;
  Log.trace("Handling Static Mesh Request ID: {}", smRequest->requestId);
  const auto [regionHandle, uploads] =
      geometryAllocator->allocate(smRequest->geometryData, transferSystem->getTransferContext());

  for (const auto& upload : uploads) {
    uploadPlan.uploadsByRequest[smRequest->requestId].push_back(upload);
  }
}

}
