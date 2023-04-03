#include "Finish.hpp"
#include "graphics/VulkanFactory.hpp"
#include <vulkan/vulkan_enums.hpp>

Finish::Finish(const RendererBaseCreateInfo& createInfo) : RendererBase(createInfo) {
   const auto renderPassCreateInfo =
       Graphics::Utils::RenderPassCreateInfo{.device = &createInfo.device,
                                             .physicalDevice = &createInfo.physicalDevice,
                                             .swapchainFormat = createInfo.swapchainFormat,
                                             .clearColor = false,
                                             .clearDepth = false,
                                             .flags = Graphics::Utils::eRenderPassBit_Last};

   renderPass = std::make_unique<vk::raii::RenderPass>(
       Graphics::Utils::colorAndDepthRenderPass(renderPassCreateInfo));

   swapchainFramebuffers = Graphics::Utils::createFramebuffers(device,
                                                               createInfo.swapchainImageViews,
                                                               *createInfo.depthImageView,
                                                               createInfo.swapchainExtent,
                                                               *renderPass);
}

void Finish::fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) {

   const vk::Rect2D screenRect = {.offset = {0, 0}, .extent = framebufferSize};

   const auto renderPassInfo =
       vk::RenderPassBeginInfo{.renderPass = *(*renderPass),
                               .framebuffer = **swapchainFramebuffers[currentImage],
                               .renderArea = screenRect};

   cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
   cmd.endRenderPass();
}

void Finish::update() {
}
