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
#include "resources/processors/Helpers.hpp"
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
          processBatchedResources(batch->batchId);
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

auto DefaultAssetSystem::extractRequirements(uint64_t batchId,
                                             const std::vector<RequestVariant>& requests)
    -> std::vector<StagingRequirements> {
  auto stagingRequirements = std::vector<StagingRequirements>{};
  for (const auto& request : requests) {
    stagingRequirements.push_back(std::visit(
        [this, batchId](const auto& req) -> StagingRequirements {
          using T = std::decay_t<decltype(*req)>;
          return resourceProcessorFactory->getProcessorFor(typeid(T))->analyze(batchId, req);
        },
        request));
  }
  return stagingRequirements;
}

auto DefaultAssetSystem::partition(BufferSizes stagingBufferSizes,
                                   const std::vector<StagingRequirements>& requirements)
    -> std::vector<SubBatch> {
  auto subBatches = std::vector<SubBatch>{};
  auto currentBatch = SubBatch{};
  size_t currentGeometryBufferSize = 0L;
  size_t currentImageBufferSize = 0L;

  for (const auto& req : requirements) {
    const auto reqGeometry = req.geometrySize.value_or(0L);
    const auto reqImage = req.imageSize.value_or(0L);

    const auto fits = (currentGeometryBufferSize + reqGeometry < stagingBufferSizes.geometry &&
                       currentImageBufferSize + reqImage < stagingBufferSizes.image);

    if (!fits && !currentBatch.items.empty()) {
      subBatches.push_back(std::move(currentBatch));
      currentBatch = SubBatch{};
      currentGeometryBufferSize = 0L;
      currentImageBufferSize = 0L;
    }

    currentBatch.items.push_back(req);
    currentGeometryBufferSize += reqGeometry;
    currentImageBufferSize += reqImage;
  }
  if (!currentBatch.items.empty()) {
    subBatches.push_back(std::move(currentBatch));
  }
  return subBatches;
}

/// Each StagingRequirement in the subBatch will produce multiple BufferUploadItems
/// and possibly multiple ImageUploadItems. This method returns a single UploadSubBatch, containing
/// all of these.
auto DefaultAssetSystem::prepareUpload(const SubBatch& subBatch) -> UploadSubBatch {
  auto uploadSubBatch = UploadSubBatch{};
  for (const auto& reqs : subBatch.items) {
    // Geometry
    if (reqs.geometrySize) {
      const auto geometryAllocation =
          geometryAllocator->allocate(*reqs.geometryData, transferSystem->getTransferContext());
      auto bufferUploadItem = GeometryUpload{.cargo = reqs.cargo,
                                             .responseType = reqs.responseType,
                                             .bufferAllocation = geometryAllocation};
      uploadSubBatch.bufferUploadItems.push_back(bufferUploadItem);
    }

    // Image(s)
    if (reqs.imageSize) {
      for (const auto& imageData : reqs.imageDataList) {
        auto byteArray =
            std::make_shared<std::vector<std::byte>>(processorHelpers::toByteVector(imageData));
        // Create a destination image
        const auto imageHandle = imageManager->createImage({
            .logicalName = "ModelTexture",
            .format = processorHelpers::getVkFormat(imageData->bits, imageData->component),
            .extent =
                vk::Extent2D{
                    .width = static_cast<uint32_t>(imageData->width),
                    .height = static_cast<uint32_t>(imageData->height),
                },
            .usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            .aspectFlags = vk::ImageAspectFlagBits::eColor,
            .debugName = "ModelTexture",
        });
        // Allocate
        auto stagingBufferOffset =
            transferSystem->getTransferContext().imageStagingAllocator->allocate(
                {.size = byteArray->size()});
        // Create ImageUpload
        auto imageUpload = ImageUpload{
            .cargo = reqs.cargo,
            .responseType = reqs.responseType,
            .data = byteArray,
            .dataSize = byteArray->size(),
            .dstImage = imageHandle,
            .subresource = vk::ImageSubresourceLayers{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                      .mipLevel = 0,
                                                      .baseArrayLayer = 0,
                                                      .layerCount = 1},
            .imageExtent = vk::Extent3D{.width = static_cast<uint32_t>(imageData->width),
                                        .height = static_cast<uint32_t>(imageData->height),
                                        .depth = 1},
            .stagingBufferOffset = stagingBufferOffset->offset,
        };
        uploadSubBatch.imageUploadItems.push_back(imageUpload);
      }
    }
  }
  return uploadSubBatch;
}

auto DefaultAssetSystem::processBatchedResources(uint64_t batchId) -> void {
  ZoneScoped;

  auto stagingRequirements = extractRequirements(batchId, eventBatches[batchId]);

  auto subBatches = partition({.geometry = transferSystem->getGeometryStagingBufferSize(),
                               .image = transferSystem->getImageStagingBufferSize()},
                              stagingRequirements);

  for (const auto& subBatch : subBatches) {
    const auto uploadSubBatch = prepareUpload(subBatch);
    const auto subBatchResults = transferSystem->upload2(uploadSubBatch);
    const auto responses = processResults(subBatchResult);
    for (const auto& response : responses) {
      std::visit(EmitEventVisitor{eventQueue}, response);
    }
  }
}

}
