#pragma once
#include "vma_raii.h"

#include <cstdint>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct RendererBaseCreateInfo {
   const vk::raii::Device& device;
   const vk::Image& depthTexture;
   const vk::Extent2D& swapchainExtent;
   const std::vector<vk::raii::Image>& swapchainImages;
};

class RendererBase {
 public:
   explicit RendererBase(const RendererBaseCreateInfo& createInfo);
   virtual ~RendererBase() = default;

   virtual void fillCommandBuffer(vk::raii::CommandBuffer&, uint32_t currentImage) = 0;

   virtual const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const = 0;
   virtual const vk::raii::DescriptorSet& getDescriptorSet(size_t currentImage) const = 0;

   const vk::Image& getDepthTexture() const {
      return depthTexture;
   }

 protected:
   void beginRenderPass(const vk::raii::CommandBuffer& commandBuffer, size_t currentImage) const;
   bool createUniformBuffers();

   const vk::raii::Device& device;
   const vk::Image& depthTexture;
   const vk::Extent2D framebufferSize{};

   std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;

   std::vector<std::unique_ptr<vk::raii::Framebuffer>> swapchainFramebuffers;

   std::unique_ptr<vk::raii::RenderPass> renderPass;
   std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
   std::unique_ptr<vk::raii::Pipeline> pipeline;

   std::vector<std::unique_ptr<vma::raii::AllocatedBuffer>> uniformBuffers;
};
