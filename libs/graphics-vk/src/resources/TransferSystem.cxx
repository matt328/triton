#include "TransferSystem.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "buffers/BufferSystem.hpp"
#include "gfx/QueueTypes.hpp"
#include "resources/ByteConverters.hpp"
#include "resources/allocators/LinearAllocator.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"

namespace tr {

constexpr size_t StagingBufferSize = 183886080;

TransferSystem::TransferSystem(std::shared_ptr<BufferSystem> newBufferSystem,
                               std::shared_ptr<Device> newDevice,
                               std::shared_ptr<queue::Transfer> newTransferQueue,
                               const std::shared_ptr<CommandBufferManager>& commandBufferManager)
    : bufferSystem{std::move(newBufferSystem)},
      device{std::move(newDevice)},
      transferQueue{std::move(newTransferQueue)},
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

auto TransferSystem::upload(UploadPlan& bufferPlan, ImageUploadPlan& imagePlan) -> void {
  ZoneScoped;
  bufferPlan.sortByBuffer();

  auto bufferResizes = checkSizes(bufferPlan);
  auto imageResizes = checkImageSizes(imagePlan);

  processResizes(bufferResizes, imageResizes);

  auto bufferCopies = prepareBufferStagingData(bufferPlan);
  auto imageCopies = prepareImageStagingData(imagePlan);

  commandBuffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  recordBufferUploads(bufferCopies);
  recordImageUploads(imageCopies);

  commandBuffer->end();

  submitAndWait();

  transferContext.stagingAllocator.reset();
  transferContext.imageStagingAllocator.reset();
}

auto TransferSystem::prepareBufferStagingData(const UploadPlan& bufferPlan) -> BufferCopyMap {
  auto bufferCopies = BufferCopyMap{};
  for (const auto& upload : bufferPlan.uploads) {
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

auto TransferSystem::recordImageUploads(const ImageUploadPlan& imagePlan) -> void {
}

auto TransferSystem::submitAndWait() -> void {
  const auto submitInfo =
      vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &**commandBuffer};

  transferQueue->getQueue().submit(submitInfo, **fence);

  if (const auto result = device->getVkDevice().waitForFences(**fence, 1u, UINT64_MAX);
      result != vk::Result::eSuccess) {
    Log.warn("Timeout waiting for fence during asnyc submit");
  }
  device->getVkDevice().resetFences(**fence);
}

auto TransferSystem::getTransferContext() -> TransferContext& {
  return transferContext;
}

auto TransferSystem::enqueueResize([[maybe_unused]] const ResizeRequest& resize) -> void {
}

auto TransferSystem::defragment([[maybe_unused]] const DefragRequest& defrag) -> void {
}

auto TransferSystem::checkSizes([[maybe_unused]] const UploadPlan& uploadPlan)
    -> std::vector<ResizeRequest> {
  // TODO(resources): Actually check sizes
  return {};
}

auto TransferSystem::checkImageSizes(const ImageUploadPlan& imagePlan)
    -> std::vector<ResizeRequest> {
  return {};
}

auto TransferSystem::processResizes(
    [[maybe_unused]] const std::vector<ResizeRequest>& resizeRequestList,
    [[maybe_unused]] const std::vector<ResizeRequest>& imageResizeRequestList) -> void {
}

}
