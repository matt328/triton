#pragma once

#include "graphics/Instance.hpp"
#include "core/vma_raii.hpp"

struct SwapchainSupportDetails {
   vk::SurfaceCapabilitiesKHR capabilities;
   std::vector<vk::SurfaceFormatKHR> formats;
   std::vector<vk::PresentModeKHR> presentModes;
};

struct FramebufferData {
   std::unique_ptr<vk::raii::ImageView> imageView;
   std::unique_ptr<vk::raii::Framebuffer> frameBuffer;
};

using ImageViewMap = std::unordered_map<std::string_view, std::unique_ptr<vk::raii::ImageView>>;
using FramebufferMap = std::unordered_map<std::string_view, std::unique_ptr<vk::raii::Framebuffer>>;

class Swapchain {
 public:
   Swapchain(const Instance& instance,
             const vk::raii::PhysicalDevice& physicalDevice,
             const vk::raii::Device& device,
             const vma::raii::Allocator& raiillocator);

   ~Swapchain() = default;

   Swapchain(const Swapchain&) = delete;
   Swapchain& operator=(const Swapchain&) = delete;
   Swapchain(Swapchain&&) = delete;
   Swapchain& operator=(Swapchain&&) = delete;

   static SwapchainSupportDetails querySwapchainSupport(
       const vk::raii::PhysicalDevice& possibleDevice,
       const std::unique_ptr<vk::raii::SurfaceKHR>& surface);

   [[nodiscard]] const vk::Format& getImageFormat() const {
      return imageFormat;
   }

   [[nodiscard]] const vk::Extent2D& getExtent() const {
      return extent;
   }

   [[nodiscard]] const vk::raii::Device& getDevice() const {
      return device;
   }

   [[nodiscard]] const vk::raii::PhysicalDevice& getPhysicalDevice() const {
      return physicalDevice;
   }

   void setCurrentFrame(uint32_t currentFrame) {
      this->currentFrame = currentFrame;
   };

   [[nodiscard]] const vk::raii::Framebuffer& getCurrentFramebuffer(const std::string& id) const;
   void createFramebuffers(const std::string& name, const vk::raii::RenderPass& renderPass);
   void destroyFramebuffers(const std::string& name);

 private:
   uint32_t currentFrame;
   ImageViewMap imageViewMap;
   FramebufferMap framebufferMap;

   std::unique_ptr<vk::raii::SwapchainKHR> swapchain;

   const vk::raii::Device& device;
   const vk::raii::PhysicalDevice& physicalDevice;

   vk::Extent2D extent;
   vk::Format imageFormat;

   std::unique_ptr<vma::raii::AllocatedImage> depthImage;
   std::unique_ptr<vk::raii::ImageView> depthImageView;

   static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
       const std::vector<vk::SurfaceFormatKHR>& availableFormats);

   static vk::PresentModeKHR chooseSwapPresentMode(
       const std::vector<vk::PresentModeKHR>& availablePresentModes);

   [[nodiscard]] vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                               const Instance& instance) const;
};
