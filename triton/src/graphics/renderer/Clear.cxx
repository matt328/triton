#include "Clear.hpp"

#include "graphics/VulkanFactory.hpp"

namespace Triton {

   using Utils::createFramebuffers;

   Clear::Clear(const RendererBaseCreateInfo& createInfo) :
       framebufferSize(createInfo.swapchainExtent) {

      const auto renderPassCreateInfo =
          Utils::RenderPassCreateInfo{.device = &createInfo.device,
                                      .physicalDevice = &createInfo.physicalDevice,
                                      .swapchainFormat = createInfo.swapchainFormat,
                                      .clearColor = true,
                                      .clearDepth = true,
                                      .flags = Utils::eRenderPassBit_First};

      renderPass = std::make_unique<vk::raii::RenderPass>(
          Utils::colorAndDepthRenderPass(renderPassCreateInfo));

      framebuffers = createFramebuffers(createInfo.device,
                                        createInfo.swapchainImageViews,
                                        *createInfo.depthImageView,
                                        createInfo.swapchainExtent,
                                        *renderPass);
   }

   void Clear::fillCommandBuffer(const vk::raii::CommandBuffer& cmd, const size_t currentImage) {
      const std::array<vk::ClearValue, 2> clearValues{
          vk::ClearValue{
              .color = vk::ClearColorValue{std::array<float, 4>({{0.39f, 0.58f, 0.93f, 1.f}})}},
          vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

      const vk::Rect2D screenRect = {.offset = {0, 0}, .extent = framebufferSize};

      const auto renderPassInfo =
          vk::RenderPassBeginInfo{.renderPass = **renderPass,
                                  .framebuffer = **framebuffers[currentImage],
                                  .renderArea = screenRect,
                                  .clearValueCount = 2,
                                  .pClearValues = clearValues.data()};

      cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
      // Don't do anything, we're just clearing the framebuffers
      cmd.endRenderPass();
   }

   void Clear::update() {
   }
}