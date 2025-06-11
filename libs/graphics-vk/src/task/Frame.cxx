#include "Frame.hpp"
#include "VkResourceManager.hpp"
#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"
#include <vulkan/vulkan_core.h>

namespace tr {

Frame::Frame(const uint8_t newIndex,
             vk::raii::Fence&& newRenderFence,
             vk::raii::Semaphore&& newImageAvailableSemaphore,
             vk::raii::Semaphore&& newRenderFinishedSemaphore,
             vk::raii::Semaphore&& newComputeFinishedSemaphore)
    : index{newIndex},
      inFlightFence{std::move(newRenderFence)},
      imageAvailableSemaphore{std::move(newImageAvailableSemaphore)},
      renderFinishedSemaphore{std::move(newRenderFinishedSemaphore)},
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

auto Frame::getObjectCount() const -> uint32_t {
  return objectCount;
}

auto Frame::addLogicalImage(LogicalHandle<ManagedImage> logicalHandle,
                            Handle<ManagedImage> imageHandle) -> void {
  assert(!imageHandles.contains(logicalHandle) &&
         "Attempted to register same logical handle twice");
  imageHandles.emplace(logicalHandle, imageHandle);
}

auto Frame::getLogicalImage(LogicalHandle<ManagedImage> logicalHandle) const
    -> Handle<ManagedImage> {
  assert(imageHandles.contains(logicalHandle));
  return imageHandles.at(logicalHandle);
}

auto Frame::addLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle,
                             Handle<ManagedBuffer> bufferHandle) -> void {
  assert(!bufferHandles.contains(logicalHandle) &&
         "Attempted to register same logical handle twice");
  bufferHandles.emplace(logicalHandle, bufferHandle);
}

[[nodiscard]] auto Frame::getLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle) const
    -> Handle<ManagedBuffer> {
  assert(bufferHandles.contains(logicalHandle));
  return bufferHandles.at(logicalHandle);
}

auto Frame::setSwapchainImageIndex(const uint32_t index) -> void {
  swapchainImageIndex = index;
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
