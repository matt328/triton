#pragma once

#include "graphics/renderer/RendererBase.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

class Terrain : public RendererBase {
 public:
   Terrain(const RendererBaseCreateInfo& createInfo);
   ~Terrain() override = default;

   Terrain(const Terrain&) = delete;
   Terrain(Terrain&&) = delete;
   Terrain& operator=(const Terrain&) = delete;
   Terrain& operator=(Terrain&&) = delete;

   void fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) override;
   void update() override;

 private:
   void createRenderPass(const vk::raii::Device* device,
                         const vk::raii::PhysicalDevice* physicalDevice,
                         const vk::Format swapchainFormat);

   void createPipeline(const vk::Extent2D swapchainExtent, const vk::raii::Device* device);

   std::shared_ptr<vk::raii::RenderPass> renderPass;
   std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
   std::unique_ptr<vk::raii::Pipeline> pipeline;
};
