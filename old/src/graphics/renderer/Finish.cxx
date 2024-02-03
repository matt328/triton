#include "Finish.hpp"
#include "graphics/VulkanFactory.hpp"

namespace Triton {

   using namespace Utils;

   Finish::Finish(const RendererBaseCreateInfo& createInfo) :
       framebufferSize(createInfo.swapchainExtent) {
      const auto renderPassCreateInfo =
          RenderPassCreateInfo{.device = &createInfo.device,
                               .physicalDevice = &createInfo.physicalDevice,
                               .swapchainFormat = createInfo.swapchainFormat,
                               .clearColor = false,
                               .clearDepth = false,
                               .flags = Utils::eRenderPassBit_Last};

      renderPass =
          std::make_unique<vk::raii::RenderPass>(colorAndDepthRenderPass(renderPassCreateInfo));

      framebuffers = createFramebuffers(createInfo.device,
                                        createInfo.swapchainImageViews,
                                        *createInfo.depthImageView,
                                        createInfo.swapchainExtent,
                                        *renderPass);
   }

   void Finish::fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) {

      const vk::Rect2D screenRect = {.offset = {0, 0}, .extent = framebufferSize};

      const auto renderPassInfo =
          vk::RenderPassBeginInfo{.renderPass = *(*renderPass),
                                  .framebuffer = **framebuffers[currentImage],
                                  .renderArea = screenRect};

      cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
      cmd.endRenderPass();
   }

   void Finish::update() {
   }
}