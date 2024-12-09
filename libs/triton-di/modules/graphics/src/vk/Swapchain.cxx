#include "Swapchain.hpp"
#include "Surface.hpp"

namespace tr::gfx {
   Swapchain::Swapchain(std::shared_ptr<PhysicalDevice> newPhysicalDevice,
                        std::shared_ptr<Device> newDevice,
                        std::shared_ptr<Surface> newSurface)
       : physicalDevice{std::move(newPhysicalDevice)},
         device{std::move(newDevice)},
         surface{std::move(newSurface)} {

      auto queueFamilyIndicesInfo = physicalDevice->getQueueFamilyIndices();

      auto [capabilities, formats, presentModes] = physicalDevice->querySwapchainSupport();

      const auto surfaceFormat = chooseSurfaceFormat(formats);
      const auto presentMode = choosePresentMode(presentModes);
      const auto extent = chooseSwapExtent(capabilities, physicalDevice->getSurfaceSize());
      // One over the min, but not if it exceeds the max
      const auto imageCount =
          std::min(capabilities.minImageCount + 1,
                   capabilities.maxImageCount > 0 ? capabilities.maxImageCount : UINT32_MAX);

      auto swapchainCreateInfo =
          vk::SwapchainCreateInfoKHR{.surface = *surface->getVkSurface(),
                                     .minImageCount = imageCount,
                                     .imageFormat = surfaceFormat.format,
                                     .imageColorSpace = surfaceFormat.colorSpace,
                                     .imageExtent = extent,
                                     .imageArrayLayers = 1,
                                     .imageUsage = vk::ImageUsageFlagBits::eColorAttachment |
                                                   vk::ImageUsageFlagBits::eTransferDst,
                                     .preTransform = capabilities.currentTransform,
                                     .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                     .presentMode = presentMode,
                                     .clipped = VK_TRUE,
                                     .oldSwapchain = VK_NULL_HANDLE};

      const auto queueFamilyIndices = std::array{queueFamilyIndicesInfo.graphicsFamily.value(),
                                                 queueFamilyIndicesInfo.presentFamily.value()};

      if (queueFamilyIndicesInfo.graphicsFamily != queueFamilyIndicesInfo.presentFamily) {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
         swapchainCreateInfo.queueFamilyIndexCount = 2;
         swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
      } else {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
      }

      swapchain = device->createSwapchain(swapchainCreateInfo);

      swapchainExtent = extent;
      swapchainImageFormat = surfaceFormat.format;

      swapchainImages = swapchain->getImages();
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
         const auto createInfo = vk::ImageViewCreateInfo{.image = image,
                                                         .viewType = vk::ImageViewType::e2D,
                                                         .format = surfaceFormat.format,
                                                         .components = components,
                                                         .subresourceRange = subresourceRange};
         swapchainImageViews.emplace_back(device->getVkDevice(), createInfo);
      }
   }

   Swapchain::~Swapchain() {
      Log.trace("Destroying Swapchain");
   }

   auto Swapchain::getImageFormat() const -> vk::Format {
      return swapchainImageFormat;
   }

   auto Swapchain::acquireNextImage(const vk::Semaphore& semaphore) const
       -> std::variant<uint32_t, ImageAcquireResult> {
      try {
         ZoneNamedN(acquire, "Acquire Swapchain Image", true);
         auto [result, imageIndex] = swapchain->acquireNextImage(UINT64_MAX, semaphore, nullptr);
         if (result == vk::Result::eSuccess) {
            return imageIndex;
         }
         if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) {
            return ImageAcquireResult::NeedsResize;
         }
         return ImageAcquireResult::Error;
      } catch (const std::exception& ex) {
         Log.warn("Swapchain needs resized: {0}", ex.what());
         return ImageAcquireResult::NeedsResize;
      }
   }

   auto Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
       -> vk::PresentModeKHR {
      for (const auto& availablePresentMode : availablePresentModes) {
         if (availablePresentMode == vk::PresentModeKHR::eFifo) {
            return availablePresentMode;
         }
      }
      return vk::PresentModeKHR::eFifo;
   }

   auto Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
       -> vk::SurfaceFormatKHR {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
             availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }

   auto Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                    const std::pair<uint32_t, uint32_t>& windowSize)
       -> vk::Extent2D {
      if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
         return capabilities.currentExtent;
      }
      const auto& [width, height] = windowSize;
      vk::Extent2D actualExtent = {width, height};

      actualExtent.width = std::clamp(actualExtent.width,
                                      capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height,
                                       capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height);

      return actualExtent;
   }
}
