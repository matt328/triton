#include "Clear.hpp"

#include "graphics/Swapchain.hpp"
#include "graphics/VulkanFactory.hpp"
#include "graphics/renderer/RendererBase.hpp"

using Graphics::Utils::createFramebuffers;

Clear::Clear(Swapchain& swapchain) : RendererBase(swapchain) {

   const auto renderPassCreateInfo =
       Graphics::Utils::RenderPassCreateInfo{.device = &swapchain.getDevice(),
                                             .physicalDevice = &swapchain.getPhysicalDevice(),
                                             .swapchainFormat = swapchain.getImageFormat(),
                                             .clearColor = true,
                                             .clearDepth = true,
                                             .flags = Graphics::Utils::eRenderPassBit_First};

   renderPass = std::make_unique<vk::raii::RenderPass>(
       Graphics::Utils::colorAndDepthRenderPass(renderPassCreateInfo));

   getSwapchain().createFramebuffers(getName(), *renderPass);
}

Clear::~Clear() {
   getSwapchain().destroyFramebuffers(getName());
}

void Clear::fillCommandBuffer(const vk::raii::CommandBuffer& cmd, const size_t currentImage) {
   const std::array<vk::ClearValue, 2> clearValues{
       vk::ClearValue{.color =
                          vk::ClearColorValue{std::array<float, 4>({{0.39f, 0.58f, 0.93f, 1.f}})}},
       vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

   const auto extent = getSwapchain().getExtent();

   // TODO maybe need to pass the currentImage in here for multithreaded rendering rather than
   // relying on the Swapchain class being set with the current one?
   const auto& framebuffer = getSwapchain().getCurrentFramebuffer(getName());

   const vk::Rect2D screenRect = {.offset = {0, 0}, .extent = extent};

   const auto renderPassInfo = vk::RenderPassBeginInfo{.renderPass = **renderPass,
                                                       .framebuffer = *framebuffer,
                                                       .renderArea = screenRect,
                                                       .clearValueCount = 2,
                                                       .pClearValues = clearValues.data()};

   cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
   // Don't do anything, we're just clearing the framebuffers
   cmd.endRenderPass();
}

void Clear::update() {
}

void Clear::resetFramebuffers(const FramebufferInfo& info) {
}
