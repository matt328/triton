#include "Frame.hpp"
#include "VkResourceManager.hpp"
#include <vulkan/vulkan_core.h>

namespace tr {

Frame::Frame(const uint8_t newIndex,
             vk::raii::Fence&& newRenderFence,
             vk::raii::Semaphore&& newImageAvailableSemaphore,
             vk::raii::Semaphore&& newRenderFinishedSemaphore,
             vk::raii::Semaphore&& newComputeFinishedSemaphore,
             CommandBufferHandle newStartCmdBuffer,
             CommandBufferHandle newEndCmdBuffer,
             CommandBufferHandle newMainCmdBuffer)
    : index{newIndex},
      inFlightFence{std::move(newRenderFence)},
      imageAvailableSemaphore{std::move(newImageAvailableSemaphore)},
      renderFinishedSemaphore{std::move(newRenderFinishedSemaphore)},
      computeFinishedSemaphore{std::move(newComputeFinishedSemaphore)},
      startCmdBuffer(newStartCmdBuffer),
      mainCmdBuffer(newMainCmdBuffer),
      endCmdBuffer(newEndCmdBuffer) {
}

auto Frame::setupRenderingInfo(const std::shared_ptr<VkResourceManager>& resourceManager) -> void {
  colorAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = resourceManager->getImageView(getDrawImageHandle()),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearValue{.color = vk::ClearColorValue{std::array<float, 4>(
                                       {{0.39f, 0.58f, 0.93f, 1.f}})}},
  };

  depthAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = resourceManager->getImageView(getDepthImageHandle()),
      .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
  };

  renderingInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0},
                               .extent = resourceManager->getImageExtent(getDrawImageHandle())},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo};
}

auto Frame::getIndexedName(std::string_view input) const -> std::string {
  return fmt::format("{0}{1}", input.data(), index);
}

auto Frame::getIndex() const -> uint8_t {
  return index;
}

auto Frame::getImageAvailableSemaphore() const -> const vk::raii::Semaphore& {
  return imageAvailableSemaphore;
}

auto Frame::getRenderFinishedSemaphore() const -> const vk::raii::Semaphore& {
  return renderFinishedSemaphore;
}

auto Frame::getComputeFinishedSemaphore() const -> const vk::raii::Semaphore& {
  return computeFinishedSemaphore;
}

auto Frame::getInFlightFence() const -> const vk::raii::Fence& {
  return inFlightFence;
}

auto Frame::getSwapchainImageIndex() const noexcept -> uint32_t {
  return swapchainImageIndex;
}

auto Frame::getDrawImageHandle() const -> ImageHandle {
  return drawImageHandle;
}

auto Frame::getRenderingInfo() const -> vk::RenderingInfo {
  return renderingInfo;
}

auto Frame::getBufferHandle(BufferHandleType type) const -> BufferHandle {
  return bufferHandleMap.at(type);
}

auto Frame::getDepthImageHandle() const -> ImageHandle {
  return depthImageHandle;
}

auto Frame::getStartCommandBufferHandle() const -> CommandBufferHandle {
  return startCmdBuffer;
}

auto Frame::getMainCommandBufferHandle() const -> CommandBufferHandle {
  return mainCmdBuffer;
}

auto Frame::getEndCommandBufferHandle() const -> CommandBufferHandle {
  return endCmdBuffer;
}

auto Frame::getStaticObjectCount() const -> uint32_t {
  return staticObjectCount;
}

auto Frame::getDynamicObjectCount() const -> uint32_t {
  return dynamicObjectCount;
}

auto Frame::getTerrainChunkCount() const -> uint32_t {
  return terrainChunkCount;
}

auto Frame::getDebugObjectCount() const -> uint32_t {
  return debugObjectCount;
}

auto Frame::addLogicalImage(LogicalImageHandle logicalHandle, ImageHandle imageHandle) -> void {
  assert(!imageHandles.contains(logicalHandle) &&
         "Attempted to register same logical handle twice");
  imageHandles.emplace(logicalHandle, imageHandle);
}

auto Frame::getLogicalImage(LogicalImageHandle logicalHandle) const -> ImageHandle {
  return imageHandles.at(logicalHandle);
}

auto Frame::addLogicalBuffer(LogicalBufferHandle logicalHandle, BufferHandle bufferHandle) -> void {
  assert(!bufferHandles.contains(logicalHandle) &&
         "Attempted to register same logical handle twice");
  bufferHandles.emplace(logicalHandle, bufferHandle);
}

[[nodiscard]] auto Frame::getLogicalBuffer(LogicalBufferHandle logicalHandle) const
    -> BufferHandle {
  return bufferHandles.at(logicalHandle);
}

auto Frame::setDepthImageHandle(const ImageHandle handle) -> void {
  depthImageHandle = handle;
}

auto Frame::setDrawImageHandle(const ImageHandle handle) -> void {
  drawImageHandle = handle;
}

auto Frame::setSwapchainImageIndex(const uint32_t index) -> void {
  swapchainImageIndex = index;
}

auto Frame::setBufferHandle(BufferHandleType type, BufferHandle handle) -> void {
  bufferHandleMap.insert({type, handle});
}

auto Frame::setStaticObjectCount(uint32_t newObjectCount) -> void {
  staticObjectCount = newObjectCount;
}

auto Frame::setDynamicObjectCount(uint32_t newObjectCount) -> void {
  dynamicObjectCount = newObjectCount;
}

auto Frame::setTerrainChunkCount(uint32_t newObjectCount) -> void {
  terrainChunkCount = newObjectCount;
}

auto Frame::setDebugObjectCount(uint32_t newDebugObjectCount) -> void {
  debugObjectCount = newDebugObjectCount;
}

auto Frame::transitionImage(const vk::raii::CommandBuffer& cmd,
                            const vk::Image& image,
                            const vk::ImageLayout currentLayout,
                            const vk::ImageLayout newLayout) -> void {
  const auto barrier = vk::ImageMemoryBarrier{
      .srcAccessMask = vk::AccessFlagBits::eMemoryWrite,
      .dstAccessMask = vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
      .oldLayout = currentLayout,
      .newLayout = newLayout,
      .image = image,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .levelCount = 1,
                           .layerCount = 1}};

  cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                      vk::PipelineStageFlagBits::eAllCommands,
                      vk::DependencyFlagBits{}, // None
                      {},
                      {},
                      barrier);
}

auto Frame::registerBuffer(BufferHandle handle) -> size_t {
  const auto key = bufferHandleKeygen.getKey();
  bufferHandleMap2.emplace(key, handle);
  return key;
}

auto Frame::getBufferHandle2(size_t key) const -> BufferHandle {
  assert(bufferHandleMap2.contains(key) && "Invalid buffer key requested");
  return bufferHandleMap2.at(key);
}

}
