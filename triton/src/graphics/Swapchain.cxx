#include "graphics/Swapchain.hpp"
#include "core/vma_raii.hpp"
#include "graphics/VulkanFactory.hpp"
#include <vulkan/vulkan_raii.hpp>

using Core::Log;

Swapchain::Swapchain(const Instance& instance,
                     const vk::raii::PhysicalDevice& physicalDevice,
                     const vk::raii::Device& device,
                     const vma::raii::Allocator& raiillocator) :
    device(device),
    physicalDevice(physicalDevice), currentFrame(0) {
   const auto& surface = instance.getSurface();
   auto [capabilities, formats, presentModes] = querySwapchainSupport(physicalDevice, surface);

   const auto surfaceFormat = chooseSwapSurfaceFormat(formats);
   const auto presentMode = chooseSwapPresentMode(presentModes);

   extent = chooseSwapExtent(capabilities, instance);

   uint32_t imageCount = capabilities.minImageCount + 1;

   if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
      imageCount = capabilities.maxImageCount;
   }

   vk::SwapchainCreateInfoKHR createInfo{.surface = **surface,
                                         .minImageCount = imageCount,
                                         .imageFormat = surfaceFormat.format,
                                         .imageColorSpace = surfaceFormat.colorSpace,
                                         .imageExtent = extent,
                                         .imageArrayLayers = 1,
                                         .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                                         .preTransform = capabilities.currentTransform,
                                         .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                         .presentMode = presentMode,
                                         .clipped = VK_TRUE,
                                         .oldSwapchain = VK_NULL_HANDLE};

   auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
       Graphics::Utils::findQueueFamilies(physicalDevice, surface);

   const auto queueFamilyIndices =
       std::array<uint32_t, 2>{graphicsFamily.value(), presentFamily.value()};

   if (graphicsFamily != presentFamily) {
      createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
   } else {
      createInfo.imageSharingMode = vk::SharingMode::eExclusive;
   }

   swapchain = std::make_unique<vk::raii::SwapchainKHR>(device, createInfo);
   Log::core->info("Created Swapchain");

   imageFormat = surfaceFormat.format;

   // Depth Image[View]
   const auto depthFormat = Graphics::Utils::findDepthFormat(physicalDevice);

   const auto imageCreateInfo = vk::ImageCreateInfo{
       .imageType = vk::ImageType::e2D,
       .format = depthFormat,
       .extent = vk::Extent3D{.width = extent.width, .height = extent.height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .samples = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
       .sharingMode = vk::SharingMode::eExclusive,
       .initialLayout = vk::ImageLayout::eUndefined};

   constexpr auto allocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

   depthImage = raiillocator.createImage(imageCreateInfo, allocationCreateInfo);

   constexpr auto range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eDepth,
                                                    .baseMipLevel = 0,
                                                    .levelCount = 1,
                                                    .baseArrayLayer = 0,
                                                    .layerCount = 1};

   const auto viewInfo = vk::ImageViewCreateInfo{.image = depthImage->getImage(),
                                                 .viewType = vk::ImageViewType::e2D,
                                                 .format = depthFormat,
                                                 .subresourceRange = range};
   depthImageView = std::make_unique<vk::raii::ImageView>(device.createImageView(viewInfo));
}

void Swapchain::createFramebuffers(const std::string& name,
                                   const vk::raii::RenderPass& renderPass) {
   const auto swapchainImages = swapchain->getImages();
   auto swapchainImageViews = std::vector<vk::raii::ImageView>{};
   swapchainImageViews.reserve(swapchainImages.size());

   constexpr vk::ComponentMapping components{.r = vk::ComponentSwizzle::eIdentity,
                                             .g = vk::ComponentSwizzle::eIdentity,
                                             .b = vk::ComponentSwizzle::eIdentity,
                                             .a = vk::ComponentSwizzle::eIdentity};

   constexpr vk::ImageSubresourceRange subresourceRange{.aspectMask =
                                                            vk::ImageAspectFlagBits::eColor,
                                                        .baseMipLevel = 0,
                                                        .levelCount = 1,
                                                        .baseArrayLayer = 0,
                                                        .layerCount = 1};

   for (const auto& image : swapchainImages) {
      vk::ImageViewCreateInfo createInfo{.image = image,
                                         .viewType = vk::ImageViewType::e2D,
                                         .format = imageFormat,
                                         .components = components,
                                         .subresourceRange = subresourceRange};

      imageViewMap.emplace(name, std::make_unique<vk::raii::ImageView>(device, createInfo));

      std::array<vk::ImageView, 2> attachments;
      attachments[0] = **imageViewMap[name];
      attachments[1] = **depthImageView;

      const auto framebufferCreateInfo =
          vk::FramebufferCreateInfo{.renderPass = *renderPass,
                                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                    .pAttachments = attachments.data(),
                                    .width = extent.width,
                                    .height = extent.height,
                                    .layers = 1};

      framebufferMap.emplace(
          name,
          std::make_unique<vk::raii::Framebuffer>(device.createFramebuffer(framebufferCreateInfo)));
   }
   Log::core->info("Created {} swapchain image views", swapchainImageViews.size());
}

void Swapchain::destroyFramebuffers(const std::string& name) {
   // Remove imageViews, framebuffers from the map at [name]
}

const vk::raii::Framebuffer& Swapchain::getCurrentFramebuffer(const std::string& id) const {
   return *framebufferMap.at(id);
}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
   for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
          availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
         return availableFormat;
      }
   }
   return availableFormats[0];
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) {
   for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
         return availablePresentMode;
      }
   }
   return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                         const Instance& instance) const {
   if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
   } else {
      const auto& [width, height] = instance.getWindowSize();

      vk::Extent2D actualExtent = {width, height};

      actualExtent.width = std::clamp(
          actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height,
                                       capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height);

      return actualExtent;
   }
}

SwapchainSupportDetails Swapchain::querySwapchainSupport(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
   SwapchainSupportDetails details;
   details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(**surface);
   details.formats = possibleDevice.getSurfaceFormatsKHR(**surface);
   details.presentModes = possibleDevice.getSurfacePresentModesKHR(**surface);
   return details;
}
