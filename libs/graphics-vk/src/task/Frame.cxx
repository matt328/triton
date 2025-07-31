#include "Frame.hpp"
#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

Frame::Frame(const uint8_t newIndex,
             vk::raii::Fence&& newRenderFence,
             vk::raii::Semaphore&& newImageAvailableSemaphore,
             vk::raii::Semaphore&& newComputeFinishedSemaphore)
    : index{newIndex},
      inFlightFence{std::move(newRenderFence)},
      imageAvailableSemaphore{std::move(newImageAvailableSemaphore)},
      computeFinishedSemaphore{std::move(newComputeFinishedSemaphore)} {
}

auto Frame::getIndexedName(std::string_view input) const -> std::string {
  return std::format("{0}{1}", input.data(), index);
}

auto Frame::getIndex() const -> uint8_t {
  return index;
}

auto Frame::getImageAvailableSemaphore() const -> const vk::raii::Semaphore& {
  return imageAvailableSemaphore;
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

auto Frame::getObjectCount() const -> uint32_t {
  return objectCount;
}

auto Frame::setLastImageUse(ImageAlias imageAlias, LastImageUse lastImageUse) -> void {
  lastImageUses.insert_or_assign(imageAlias, lastImageUse);
}

auto Frame::getLastImageUse(ImageAlias imageAlias) const -> std::optional<LastImageUse> {
  if (!lastImageUses.contains(imageAlias)) {
    return std::nullopt;
  }
  return lastImageUses.at(imageAlias);
}

auto Frame::setLastBufferUse(BufferAliasVariant bufferAlias, LastBufferUse lastBufferUse) -> void {
  lastBufferUses.insert_or_assign(bufferAlias, lastBufferUse);
}

auto Frame::getLastBufferUse(BufferAliasVariant bufferAlias) const -> std::optional<LastBufferUse> {
  if (!lastBufferUses.contains(bufferAlias)) {
    return std::nullopt;
  }
  return lastBufferUses.at(bufferAlias);
}

auto Frame::setEditorState(std::optional<EditorState> newState) -> void {
  editorState = std::move(newState);
}

auto Frame::getEditorState() const -> std::optional<EditorState> {
  return editorState;
}

auto Frame::setImageTransitionInfo(const std::vector<ImageTransitionInfo>& transitionInfo) -> void {
  this->imageTransitionInfo = transitionInfo;
}

auto Frame::getImageTransitionInfo() -> std::vector<ImageTransitionInfo> {
  return imageTransitionInfo;
}

auto Frame::addLogicalImage(LogicalHandle<ManagedImage> logicalHandle,
                            Handle<ManagedImage> imageHandle) -> void {
  assert(!imageHandles.contains(logicalHandle) &&
         "Attempted to register same logical handle twice");
  imageHandles.emplace(logicalHandle, imageHandle);
}

auto Frame::registerSwapchainLogicalHandle(LogicalHandle<ManagedImage> logicalHandle) -> void {
  Log.trace("Frame registering swapchain logicalHandle={}", logicalHandle.id);
  this->swapchainLogicalHandle = logicalHandle;
}

auto Frame::addSwapchainImage(Handle<ManagedImage> handle, uint32_t index) -> void {
  Log.trace("Frame {} adding swapchain image, handle={}, index={}", this->index, handle.id, index);
  swapchainImageHandles.emplace(index, handle);
}

auto Frame::getLogicalImage(LogicalHandle<ManagedImage> logicalHandle) const
    -> Handle<ManagedImage> {
  if (logicalHandle == swapchainLogicalHandle) {
    return swapchainImageHandles.at(swapchainImageIndex);
  }
  assert(imageHandles.contains(logicalHandle));
  return imageHandles.at(logicalHandle);
}

auto Frame::addLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle,
                             Handle<ManagedBuffer> bufferHandle) -> void {
  assert(!bufferHandles.contains(logicalHandle) &&
         "Attempted to register same logical handle twice");
  bufferHandles.emplace(logicalHandle, bufferHandle);
}

auto Frame::addLogicalShaderBinding(LogicalHandle<IShaderBinding> logicalHandle,
                                    Handle<IShaderBinding> handle) -> void {
  assert(!shaderBindingHandles.contains(logicalHandle) &&
         "Attempted to register same logical ShaderBinding twice");
  shaderBindingHandles.emplace(logicalHandle, handle);
}

[[nodiscard]] auto Frame::getLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle) const
    -> Handle<ManagedBuffer> {
  assert(bufferHandles.contains(logicalHandle));
  return bufferHandles.at(logicalHandle);
}

[[nodiscard]] auto Frame::getLogicalShaderBinding(LogicalHandle<IShaderBinding> logicalHandle) const
    -> Handle<IShaderBinding> {
  assert(shaderBindingHandles.contains(logicalHandle));
  return shaderBindingHandles.at(logicalHandle);
}

auto Frame::setSwapchainImageIndex(const uint32_t index) -> void {
  swapchainImageIndex = index;
}

auto Frame::setObjectCount(uint32_t newObjectCount) -> void {
  objectCount = newObjectCount;
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

}
