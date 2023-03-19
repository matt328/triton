#include "Clear.h"

Clear::Clear(const RendererBaseCreateInfo& createInfo) : RendererBase(createInfo) {}

void Clear::fillCommandBuffer(vk::raii::CommandBuffer& cmd, const size_t currentImage) {
  std::array<vk::ClearValue, 2> clearValues;
  const std::array clearColor = {0.39f, 0.58f, 0.93f, 1.f};
  clearValues[0].color = vk::ClearColorValue(clearColor);
  clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

  const vk::Rect2D screenRect = {.offset = {0, 0}, .extent = framebufferSize};

  const auto renderPassInfo = vk::RenderPassBeginInfo{
      .renderPass = **renderPass,
      .framebuffer = **swapchainFramebuffers[currentImage],
      .renderArea = screenRect,
      .clearValueCount = 2,
      .pClearValues = clearValues.data()};

  cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
  // Don't do anything, we're just clearing the framebuffers
  cmd.endRenderPass();
}
