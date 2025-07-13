#include "DefaultAssetSystem.hpp"
#include "api/fx/IAssetService.hpp"
#include "api/fx/IEventQueue.hpp"
#include "bk/ThreadName.hpp"
#include "buffers/BufferSystem.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "img/ImageManager.hpp"
#include "img/TextureArena.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "resources/processors/IResourceProcessor.hpp"
#include "resources/processors/IResourceProcessorFactory.hpp"

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
    std::shared_ptr<TextureArena> newTextureArena,
    std::shared_ptr<IResourceProcessorFactory> newResourceProcessorFactory)
    : eventQueue{std::move(newEventQueue)},
      assetService{std::move(newAssetService)},
      bufferSystem{std::move(newBufferSystem)},
      geometryBufferPack{std::move(newGeometryBufferPack)},
      transferSystem{std::move(newTransferSystem)},
      geometryAllocator{std::move(newGeometryAllocator)},
      geometryHandleMapper{std::move(newGeometryHandleMapper)},
      textureHandleMapper{std::move(newTextureHandleMapper)},
      imageManager{std::move(newImageManager)},
      textureArena{std::move(newTextureArena)},
      resourceProcessorFactory{std::move(newResourceProcessorFactory)} {
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

  for (auto& eventVariant : eventBatches[batchId]) {
    const auto result = std::visit(
        [this](const auto& req) -> ProcessingResult {
          using T = std::decay_t<decltype(*req)>;
          return resourceProcessorFactory->getProcessorFor(typeid(T))->process(req);
        },
        eventVariant);

    uploadPlan.geometryDataByRequest.emplace(result.requestId, *result.geometryHandle);
    for (const auto& up : result.geometryUploads) {
      uploadPlan.uploadsByRequest[result.requestId].push_back(up);
    }

    for (const auto& up : result.imageUploads) {
      imageUploadPlan.uploadsByRequest[result.requestId].push_back(up);
    }

    inFlightUploads.emplace(
        result.requestId,
        InFlightUpload{.requestId = result.requestId,
                       .remainingComponents = {ComponentType::Image, ComponentType::Geometry},
                       .responseEvent = result.responseEvent});
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

      const auto handle = textureArena->insert(image.getImageView(), sampler);

      const auto textureHandle = textureHandleMapper->toPublic(handle);
      // Set the texture in the *Uploaded event
      auto& inFlight = inFlightUploads.at(requestId);
      std::visit(ImageUploadPlan::ResponseEventVisitor{textureHandle}, inFlight.responseEvent);
      inFlight.remainingComponents.erase(ComponentType::Image);
      if (inFlight.remainingComponents.empty()) {
        std::visit(EmitEventVisitor{eventQueue}, inFlight.responseEvent);
        inFlightUploads.erase(requestId);
      }
    }
  }

  // Update in flight component counts
  for (const auto& [requestId, bufferUpload] : uploadPlan.uploadsByRequest) {
    auto& inFlight = inFlightUploads.at(requestId);
    inFlight.remainingComponents.erase(ComponentType::Geometry);
    if (inFlight.remainingComponents.empty()) {
      std::visit(EmitEventVisitor{eventQueue}, inFlight.responseEvent);
      inFlightUploads.erase(requestId);
    }
  }
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
