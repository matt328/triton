#include "Frame.hpp"

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
  drawImageName = "DrawImage Frame" + std::to_string(index);
}

auto Frame::setupRenderingInfo(const std::shared_ptr<VkResourceManager>& resourceManager) -> void {
  colorAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = resourceManager->getImageView(getDrawImageId()),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearValue{.color = vk::ClearColorValue{std::array<float, 4>(
                                       {{0.39f, 0.58f, 0.93f, 1.f}})}},
  };

  depthAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = resourceManager->getImageView(DepthImageName),
      .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
  };

  renderingInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0},
                               .extent = resourceManager->getImageExtent(getDrawImageId())},
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

auto Frame::getImageAvailableSemaphore() -> vk::raii::Semaphore& {
  return imageAvailableSemaphore;
}

auto Frame::getRenderFinishedSemaphore() -> vk::raii::Semaphore& {
  return renderFinishedSemaphore;
}

auto Frame::getComputeFinishedSemaphore() -> vk::raii::Semaphore& {
  return computeFinishedSemaphore;
}

auto Frame::getInFlightFence() -> vk::raii::Fence& {
  return inFlightFence;
}

auto Frame::getSwapchainImageIndex() const -> uint32_t {
  return swapchainImageIndex;
}

auto Frame::getDrawImageId() const -> std::string {
  return drawImageName;
}

auto Frame::getRenderingInfo() const -> vk::RenderingInfo {
  return renderingInfo;
}

auto Frame::setSwapchainImageIndex(const uint32_t index) -> void {
  swapchainImageIndex = index;
}

auto Frame::addCommandBuffer(CmdBufferType cmdType, CommandBufferPtr&& commandBuffer) -> void {
  commandBuffers.insert(std::make_pair(cmdType, std::move(commandBuffer)));
}

auto Frame::getCommandBuffer(const CmdBufferType cmdType) const -> vk::raii::CommandBuffer& {
  return *commandBuffers.at(cmdType);
}

auto Frame::clearCommandBuffers() -> void {
  commandBuffers.clear();
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
