#pragma once

#include "core/vma_raii.hpp"

struct RendererBaseCreateInfo {
   const vk::raii::Device& device;
   const vk::raii::PhysicalDevice& physicalDevice;
   const vma::raii::Allocator& allocator;
   const vk::Image& depthTexture;
   const vk::Extent2D& swapchainExtent;
   const std::vector<vk::Image>& swapchainImages;
   const std::vector<vk::raii::ImageView>& swapchainImageViews;
   const vk::raii::ImageView& depthImageView;
   const vk::Format swapchainFormat;
};

class RendererBase {
 public:
   explicit RendererBase() = default;
   virtual ~RendererBase() = default;

   RendererBase(const RendererBase&) = delete;
   RendererBase(RendererBase&&) = delete;
   RendererBase& operator=(const RendererBase&) = delete;
   RendererBase& operator=(RendererBase&&) = delete;

   virtual void fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) = 0;
   virtual void update() = 0;
};
