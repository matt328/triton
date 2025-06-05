#include "TransferSystem.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "buffers/BufferSystem.hpp"
#include "gfx/QueueTypes.hpp"
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
}

auto TransferSystem::upload(UploadPlan& uploadPlan) -> void {
  uploadPlan.sortByBuffer();
  const auto resizeList = checkSizes(uploadPlan);
  if (!resizeList.empty()) {
    processResizes(resizeList);
  }
  std::unordered_map<Handle<ManagedBuffer>, std::vector<vk::BufferCopy2>> bufferCopies{};

  for (const auto& upload : uploadPlan.uploads) {
    Log.trace("Copying into staging buffer offset={}, size={}, dstBuffer={}",
              upload.stagingOffset,
              upload.dataSize,
              upload.dstBuffer.id);

    std::shared_ptr<std::vector<GpuIndexData>> testVector =
        std::static_pointer_cast<std::vector<GpuIndexData>>(upload.data);
    Log.trace("testVector.size()={}", testVector->size());

    const auto stagingBufferRegion =
        bufferSystem->insert(transferContext.stagingBuffer,
                             upload.data.get(),
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

  constexpr vk::CommandBufferBeginInfo cmdBeginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer->begin(cmdBeginInfo);

  for (const auto& [dstHandle, regions] : bufferCopies) {
    const auto copyInfo2 = vk::CopyBufferInfo2{
        .srcBuffer = *bufferSystem->getVkBuffer(transferContext.stagingBuffer).value(),
        .dstBuffer = *bufferSystem->getVkBuffer(dstHandle).value(),
        .regionCount = static_cast<uint32_t>(regions.size()),
        .pRegions = regions.data()};
    commandBuffer->copyBuffer2(copyInfo2);
  }

  commandBuffer->end();

  const auto submitInfo =
      vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &**commandBuffer};

  transferQueue->getQueue().submit(submitInfo, **fence);

  if (const auto result = device->getVkDevice().waitForFences(**fence, 1u, UINT64_MAX);
      result != vk::Result::eSuccess) {
    Log.warn("Timeout waiting for fence during asnyc submit");
  }
  device->getVkDevice().resetFences(**fence);
  transferContext.stagingAllocator.reset();
}

auto TransferSystem::getTransferContext() -> TransferContext& {
  return transferContext;
}

auto TransferSystem::enqueueResize(const ResizeRequest& resize) -> void {
}

auto TransferSystem::defragment(const DefragRequest& defrag) -> void {
}

auto TransferSystem::checkSizes([[maybe_unused]] const UploadPlan& uploadPlan)
    -> std::vector<ResizeRequest> {
  // TODO(resources): Actually check sizes
  return {};
}

auto TransferSystem::processResizes(const std::vector<ResizeRequest>& resizeRequestList) -> void {
}

}
