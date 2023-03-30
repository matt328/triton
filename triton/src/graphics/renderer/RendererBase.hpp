#pragma once

#include "core/vma_raii.hpp"

struct RendererBaseCreateInfo {
   const vk::raii::Device& device;
   const vma::raii::Allocator& allocator;
   const vk::Image& depthTexture;
   const vk::Extent2D& swapchainExtent;
   const std::vector<vk::Image>& swapchainImages;
};

class RendererBase {
 public:
   explicit RendererBase(const RendererBaseCreateInfo& createInfo);
   virtual ~RendererBase() = default;

   RendererBase(const RendererBase&) = delete;
   RendererBase(RendererBase&&) = delete;
   RendererBase& operator=(const RendererBase&) = delete;
   RendererBase& operator=(RendererBase&&) = delete;

   virtual void fillCommandBuffer(vk::raii::CommandBuffer&, size_t currentImage) = 0;

   [[nodiscard]] const vk::Image& getDepthTexture() const {
      return depthTexture;
   }

 protected:
   // These are utils that derived classes may or may not need to use.
   void beginRenderPass(const vk::raii::CommandBuffer& commandBuffer,
                        size_t currentImage,
                        const vk::raii::DescriptorSet& currentDescriptorSet) const;
   void createUniformBuffers(size_t bufferSize);

   const vk::raii::Device& device;
   const vma::raii::Allocator& allocator;
   const vk::Image& depthTexture;
   const vk::Extent2D framebufferSize{};
   const size_t framesInFlight = 0;
   const std::vector<vk::Image>& swapchainImages;

   std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;

   // Framebuffers are cheap and only are a way to access the swap chain images via an attachment
   // each renderer will get it's own set of framebuffers.
   // Need to figure out how renderers can access the swapchain images in order to produce
   // framebuffers
   std::vector<std::unique_ptr<vk::raii::Framebuffer>> swapchainFramebuffers;

   const std::unique_ptr<vk::raii::RenderPass> renderPass;
   std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
   std::unique_ptr<vk::raii::Pipeline> pipeline;

   std::vector<std::unique_ptr<vma::raii::AllocatedBuffer>> uniformBuffers;
};
