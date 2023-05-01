#pragma once

#include "core/vma_raii.hpp"
#include "graphics/Swapchain.hpp"

struct FramebufferInfo {
   const vk::raii::Device& device;
   const std::vector<vk::raii::ImageView>& swapchainImageViews;
   const vk::raii::ImageView& depthImageView;
   const vk::Extent2D& swapchainExtent;
};

struct RendererBaseCreateInfo {
   const vk::Format swapchainFormat;
   const vk::raii::PhysicalDevice& physicalDevice;
   const vma::raii::Allocator& allocator;
   const vk::Image& depthTexture;
   const std::vector<vk::Image>& swapchainImages;
   const FramebufferInfo& framebufferInfo;
};

class RendererBase {
 public:
   RendererBase(Swapchain& swapchain) : swapchain(swapchain){};
   virtual ~RendererBase() = default;

   RendererBase(const RendererBase&) = delete;
   RendererBase(RendererBase&&) = delete;
   RendererBase& operator=(const RendererBase&) = delete;
   RendererBase& operator=(RendererBase&&) = delete;

   [[nodiscard]] Swapchain& getSwapchain() const {
      return swapchain;
   }

   [[nodiscard]] virtual const std::string getName() const = 0;

   virtual void fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) = 0;
   virtual void update() = 0;

   virtual void resetFramebuffers(const FramebufferInfo& info) = 0;

 private:
   Swapchain& swapchain;
};
