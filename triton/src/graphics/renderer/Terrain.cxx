#include "graphics/renderer/Terrain.hpp"

#include "graphics/renderer/RendererBase.hpp"
#include "graphics/VulkanFactory.hpp"

Terrain::Terrain(const RendererBaseCreateInfo& createInfo) : RendererBase(createInfo) {
   createRenderPass(&createInfo.device, &createInfo.physicalDevice, createInfo.swapchainFormat);
}

Terrain::~Terrain() = default;

void Terrain::fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) {
}

void Terrain::update() {
}

void Terrain::createRenderPass(const vk::raii::Device* device,
                               const vk::raii::PhysicalDevice* physicalDevice,
                               const vk::Format swapchainFormat) {
   const auto renderPassCreateInfo =
       Graphics::Utils::RenderPassCreateInfo{.device = device,
                                             .physicalDevice = physicalDevice,
                                             .swapchainFormat = swapchainFormat,
                                             .clearColor = false,
                                             .clearDepth = false};

   renderPass = std::make_unique<vk::raii::RenderPass>(
       Graphics::Utils::colorAndDepthRenderPass(renderPassCreateInfo));
}

void Terrain::createPipeline() {
}