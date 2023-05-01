#pragma once

#include "graphics/renderer/RendererBase.hpp"

class ModelRenderer : public RendererBase {
 public:
   ModelRenderer(const RendererBaseCreateInfo& createInfo);

   ModelRenderer(const ModelRenderer&) = delete;
   ModelRenderer(ModelRenderer&&) = delete;
   ModelRenderer& operator=(const ModelRenderer&) = delete;
   ModelRenderer& operator=(ModelRenderer&&) = delete;

   ~ModelRenderer() override = default;

   void fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) override;
   void update() override;
   void resetFramebuffers(const FramebufferInfo& info) override;

 private:
   std::vector<std::unique_ptr<vk::raii::Framebuffer>> framebuffers;
   vk::Extent2D framebufferSize;
   std::unique_ptr<vk::raii::RenderPass> renderPass;
   std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
   std::unique_ptr<vk::raii::Pipeline> pipeline;

   void createRenderPass(const vk::raii::Device* device,
                         const vk::raii::PhysicalDevice* physicalDevice,
                         const vk::Format swapchainFormat);

   void createPipeline(const vk::Extent2D swapchainExtent, const vk::raii::Device* device);
};
