#include "TransferSystem.hpp"
#include "ImageTransitionQueue.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "buffers/BufferSystem.hpp"
#include "gfx/QueueTypes.hpp"
#include "img/ImageManager.hpp"
#include "img/TextureArena.hpp"
#include "resources/ByteConverters.hpp"
#include "resources/allocators/LinearAllocator.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"

namespace tr {

constexpr size_t StagingBufferSize = 183886080;

TransferSystem::TransferSystem(std::shared_ptr<BufferSystem> newBufferSystem,
                               std::shared_ptr<Device> newDevice,
                               std::shared_ptr<queue::Transfer> newTransferQueue,
                               std::shared_ptr<queue::Graphics> newGraphicsQueue,
                               std::shared_ptr<ImageManager> newImageManager,
                               std::shared_ptr<ImageTransitionQueue> newImageQueue,
                               std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
                               std::shared_ptr<TextureArena> newTextureArena,
                               std::shared_ptr<TextureHandleMapper> newTextureHandleMapper,
                               const std::shared_ptr<CommandBufferManager>& commandBufferManager)
    : bufferSystem{std::move(newBufferSystem)},
      device{std::move(newDevice)},
      transferQueue{std::move(newTransferQueue)},
      graphicsQueue{std::move(newGraphicsQueue)},
      imageManager{std::move(newImageManager)},
      imageQueue{std::move(newImageQueue)},
      geometryHandleMapper{std::move(newGeometryHandleMapper)},
      textureArena{std::move(newTextureArena)},
      textureHandleMapper{std::move(newTextureHandleMapper)},
      commandBuffer{std::make_unique<vk::raii::CommandBuffer>(
          commandBufferManager->getTransferCommandBuffer())},
      fence(std::make_unique<vk::raii::Fence>(
          device->getVkDevice().createFence(vk::FenceCreateInfo{}))) {
  transferContext.stagingBuffer =
      bufferSystem->registerBuffer(BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                                                    .bufferUsage = BufferUsage::Transfer,
                                                    .initialSize = StagingBufferSize,
                                                    .debugName = "Buffer-GeometryStaging"});
  transferContext.stagingAllocator = std::make_unique<LinearAllocator>(StagingBufferSize);

  transferContext.imageStagingBuffer =
      bufferSystem->registerBuffer(BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                                                    .bufferUsage = BufferUsage::Transfer,
                                                    .initialSize = StagingBufferSize,
                                                    .debugName = "Buffer-ImageStaging"});
  transferContext.imageStagingAllocator = std::make_unique<LinearAllocator>(StagingBufferSize);
}

auto TransferSystem::upload2(const UploadSubBatch& subBatch) -> std::vector<SubBatchResult> {
  ZoneScoped;
  auto subBatchResults = std::vector<SubBatchResult>{};

  auto [bufferResizes, imageResizes] = checkSizes(subBatch);
  processResizes(bufferResizes, imageResizes);

  auto [bufferCopies, imageCopies] = prepareStagingData(subBatch);

  commandBuffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  recordBufferUploads(bufferCopies);
  recordImageUploads(imageCopies);

  commandBuffer->end();

  submitAndWait();

  transferContext.stagingAllocator->reset();
  transferContext.imageStagingAllocator->reset();

  auto resultsMap = std::unordered_map<uint64_t, SubBatchResult>{};
  for (const auto& geometryUpload : subBatch.bufferUploadItems) {
    const auto subBatchResult = SubBatchResult{.cargo = geometryUpload.cargo,
                                               .responseType = geometryUpload.responseType,
                                               .geometryHandle = geometryHandleMapper->toPublic(
                                                   geometryUpload.bufferAllocation.regionHandle)};
    resultsMap.emplace(geometryUpload.cargo.requestId, subBatchResult);
  }
  for (const auto& imageUpload : subBatch.imageUploadItems) {
    const auto& image = imageManager->getImage(imageUpload.dstImage);
    const auto& sampler = imageManager->getSampler(imageManager->getDefaultSampler());
    auto handle = textureArena->insert(image.getImageView(), sampler);
    auto textureHandle = textureHandleMapper->toPublic(handle);
    resultsMap.at(imageUpload.cargo.requestId).textureHandle = textureHandle;
  }
  for (const auto& [_, value] : resultsMap) {
    subBatchResults.push_back(value);
  }
  return subBatchResults;
}

auto TransferSystem::prepareStagingData(const UploadSubBatch& uploadSubBatch)
    -> std::tuple<BufferCopyMap, ImageCopyMap> {
  auto bufferCopies = BufferCopyMap{};
  for (const auto& upload : uploadSubBatch.bufferUploadItems) {
    for (const auto& allocation : upload.bufferAllocation.bufferAllocations) {
      Log.trace("Copying into staging buffer offset={}, size={}, dstBuffer={}",
                allocation.stagingOffset,
                allocation.dataSize,
                allocation.dstBuffer.id);
      const auto stagingBufferRegion = bufferSystem->insert(
          transferContext.stagingBuffer,
          allocation.data->data(),
          BufferRegion{.offset = allocation.stagingOffset, .size = allocation.dataSize});

      const auto region = vk::BufferCopy2{.srcOffset = stagingBufferRegion->offset,
                                          .dstOffset = allocation.dstOffset,
                                          .size = allocation.dataSize};
      Log.trace("Created copy region, srcOffset={}, dstOffset={}, dstBuffer={}",
                stagingBufferRegion->offset,
                allocation.dstOffset,
                allocation.dstBuffer.id);
      bufferCopies[allocation.dstBuffer].push_back(region);
    }
  }

  auto imageCopies = ImageCopyMap{};
  for (const auto& imageUpload : uploadSubBatch.imageUploadItems) {
    const auto region = bufferSystem->insert(
        transferContext.imageStagingBuffer,
        imageUpload.data->data(),
        BufferRegion{.offset = imageUpload.stagingBufferOffset, .size = imageUpload.dataSize});

    const auto copyInfo = vk::BufferImageCopy2{
        .bufferOffset = region->offset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = imageUpload.subresource,
        .imageOffset = imageUpload.imageOffset,
        .imageExtent = imageUpload.imageExtent,
    };
    imageCopies.emplace(imageUpload.dstImage, std::vector<vk::BufferImageCopy2>{copyInfo});
  }

  return {bufferCopies, imageCopies};
}

auto TransferSystem::prepareBufferStagingData(const UploadPlan& bufferPlan) -> BufferCopyMap {
  auto bufferCopies = BufferCopyMap{};
  for (const auto& upload : bufferPlan.getSortedUploads()) {
    Log.trace("Copying into staging buffer offset={}, size={}, dstBuffer={}",
              upload.stagingOffset,
              upload.dataSize,
              upload.dstBuffer.id);

    const auto stagingBufferRegion =
        bufferSystem->insert(transferContext.stagingBuffer,
                             upload.data->data(),
                             BufferRegion{.offset = upload.stagingOffset, .size = upload.dataSize});
    const auto region = vk::BufferCopy2{.srcOffset = stagingBufferRegion->offset,
                                        .dstOffset = upload.dstOffset,
                                        .size = upload.dataSize};
    Log.trace("Created copy region, srcOffset={}, dstOffset={}, dstBuffer={}",
              stagingBufferRegion->offset,
              upload.dstOffset,
              upload.dstBuffer.id);
    bufferCopies[upload.dstBuffer].push_back(region);
  }
  return bufferCopies;
}

auto TransferSystem::prepareImageStagingData(const ImageUploadPlan& imagePlan) -> ImageCopyMap {
  auto imageCopies = ImageCopyMap{};
  for (const auto& [_, imageUploads] : imagePlan.uploadsByRequest) {
    for (const auto& imageUpload : imageUploads) {
      const auto region = bufferSystem->insert(
          transferContext.imageStagingBuffer,
          imageUpload.data->data(),
          BufferRegion{.offset = imageUpload.stagingBufferOffset, .size = imageUpload.dataSize});

      const auto copyInfo = vk::BufferImageCopy2{
          .bufferOffset = region->offset,
          .bufferRowLength = 0,
          .bufferImageHeight = 0,
          .imageSubresource = imageUpload.subresource,
          .imageOffset = imageUpload.imageOffset,
          .imageExtent = imageUpload.imageExtent,
      };
      imageCopies.emplace(imageUpload.dstImage, std::vector<vk::BufferImageCopy2>{copyInfo});
    }
  }
  return imageCopies;
}

auto TransferSystem::recordBufferUploads(const BufferCopyMap& bufferCopies) -> void {
  for (const auto& [dstHandle, regions] : bufferCopies) {
    const auto srcBuffer = bufferSystem->getVkBuffer(transferContext.stagingBuffer);
    const auto dstBuffer = bufferSystem->getVkBuffer(dstHandle);
    if (srcBuffer.has_value() && dstBuffer.has_value()) {
      const auto copyInfo2 =
          vk::CopyBufferInfo2{.srcBuffer = *srcBuffer.value(),
                              .dstBuffer = *dstBuffer.value(),
                              .regionCount = static_cast<uint32_t>(regions.size()),
                              .pRegions = regions.data()};
      commandBuffer->copyBuffer2(copyInfo2);
    } else {
      if (!srcBuffer.has_value()) {
        Log.warn("transferContext staging buffer could not be resolved");
      }
      if (!dstBuffer.has_value()) {
        Log.warn("transfer destination buffer could not be resolved");
      }
    }
  }
}

auto TransferSystem::recordImageUploads(const ImageCopyMap& imageCopies) -> void {
  const auto srcBuffer = bufferSystem->getVkBuffer(transferContext.imageStagingBuffer);
  for (const auto& [dstImageHandle, regions] : imageCopies) {
    const auto& dstImage = imageManager->getImage(dstImageHandle);
    vk::ImageMemoryBarrier2 imageBarrier = {
        .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
        .srcAccessMask = vk::AccessFlagBits2::eNone,
        .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
        .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eTransferDstOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = dstImage.getImage(),
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};

    commandBuffer->pipelineBarrier2(
        {.imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &imageBarrier});
    commandBuffer->copyBufferToImage2({.srcBuffer = *srcBuffer.value(),
                                       .dstImage = dstImage.getImage(),
                                       .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
                                       .regionCount = static_cast<uint32_t>(regions.size()),
                                       .pRegions = regions.data()});
    vk::ImageMemoryBarrier2 releaseBarrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .oldLayout = vk::ImageLayout::eTransferDstOptimal,
        .newLayout = vk::ImageLayout::eTransferDstOptimal, // layout stays the same
        .srcQueueFamilyIndex = transferQueue->getFamily(),
        .dstQueueFamilyIndex = graphicsQueue->getFamily(),
        .image = dstImage.getImage(),
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = regions.front().imageSubresource.aspectMask,
            .baseMipLevel = regions.front().imageSubresource.mipLevel,
            .levelCount = 1,
            .baseArrayLayer = regions.front().imageSubresource.baseArrayLayer,
            .layerCount = regions.front().imageSubresource.layerCount,
        }};
    commandBuffer->pipelineBarrier2(
        {.imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &releaseBarrier});
    transitionBatch.push_back(
        ImageTransitionInfo{.image = dstImage.getImage(),
                            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                            .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                            .subresourceRange = {
                                .aspectMask = regions.front().imageSubresource.aspectMask,
                                .baseMipLevel = regions.front().imageSubresource.mipLevel,
                                .levelCount = 1,
                                .baseArrayLayer = regions.front().imageSubresource.baseArrayLayer,
                                .layerCount = regions.front().imageSubresource.layerCount,
                            }});
  }
}

auto TransferSystem::submitAndWait() -> void {
  const auto submitInfo =
      vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &**commandBuffer};

  transferQueue->getQueue().submit(submitInfo, **fence);

  if (const auto result = device->getVkDevice().waitForFences(**fence, 1u, UINT64_MAX);
      result != vk::Result::eSuccess) {
    Log.warn("Timeout waiting for fence during asnyc submit");
  }

  // Add all the images that were uploaded to the queue here
  imageQueue->enqueue(transitionBatch);

  device->getVkDevice().resetFences(**fence);
}

auto TransferSystem::getTransferContext() -> TransferContext& {
  return transferContext;
}

auto TransferSystem::enqueueResize([[maybe_unused]] const ResizeRequest& resize) -> void {
}

auto TransferSystem::defragment([[maybe_unused]] const DefragRequest& defrag) -> void {
}

auto TransferSystem::checkSizes([[maybe_unused]] const UploadSubBatch& uploadSubBatch)
    -> std::tuple<std::vector<ResizeRequest>, std::vector<ResizeRequest>> {
  // TODO(resources): Actually check sizes
  return {};
}

auto TransferSystem::processResizes(
    [[maybe_unused]] const std::vector<ResizeRequest>& resizeRequestList,
    [[maybe_unused]] const std::vector<ResizeRequest>& imageResizeRequestList) -> void {
}

auto TransferSystem::getGeometryStagingBufferSize() -> size_t {
  return StagingBufferSize;
}
auto TransferSystem::getImageStagingBufferSize() -> size_t {
  return StagingBufferSize;
}

}
