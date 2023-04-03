#include "graphics/renderer/Terrain.hpp"

#include "graphics/renderer/RendererBase.hpp"
#include "graphics/VulkanFactory.hpp"

Terrain::Terrain(const RendererBaseCreateInfo& createInfo) : RendererBase(createInfo) {
   const auto renderPassCreateInfo =
       Graphics::Utils::RenderPassCreateInfo{.device = createInfo.device,
                                             .physicalDevice = createInfo.physicalDevice,
                                             .swapchainFormat = createInfo.swapchainFormat,
                                             .clearColor = false,
                                             .clearDepth = false};

   renderPass = std::make_unique<vk::raii::RenderPass>(
       Graphics::Utils::colorAndDepthRenderPass(renderPassCreateInfo));
}

Terrain::~Terrain() = default;

void Terrain::fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) {
}

void Terrain::update() {
}
