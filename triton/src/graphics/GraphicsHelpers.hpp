#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace graphics {

   // Structs
   struct QueueFamilyIndices {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;
      std::optional<uint32_t> transferFamily;
      std::optional<uint32_t> computeFamily;

      [[nodiscard]] bool isComplete() const {
         return graphicsFamily.has_value() && presentFamily.has_value() &&
                transferFamily.has_value() && computeFamily.has_value();
      }
   };

   struct SwapchainSupportDetails {
      vk::SurfaceCapabilitiesKHR capabilities;
      std::vector<vk::SurfaceFormatKHR> formats;
      std::vector<vk::PresentModeKHR> presentModes;
   };

   vk::PresentModeKHR chooseSwapPresentMode(
       const std::vector<vk::PresentModeKHR>& availablePresentModes) {
      for (const auto& availablePresentMode : availablePresentModes) {
         if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
         }
      }
      return vk::PresentModeKHR::eFifo;
   }

   vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
       const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
             availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }

   vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                 const Instance& instance) {
      if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
         return capabilities.currentExtent;
      } else {
         const auto& [width, height] = instance.getWindowSize();

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

   QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& possibleDevice,
                                        const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
      QueueFamilyIndices queueFamilyIndices;

      const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

      for (int i = 0; const auto& queueFamily : queueFamilies) {
         if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;
         }

         if (possibleDevice.getSurfaceSupportKHR(i, **surface)) {
            queueFamilyIndices.presentFamily = i;
         }

         if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            queueFamilyIndices.transferFamily = i;
         }

         if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
            queueFamilyIndices.computeFamily = i;
         }

         if (queueFamilyIndices.isComplete()) {
            break;
         }
         i++;
      }
      return queueFamilyIndices;
   }

   bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                    const std::vector<const char*> desiredDeviceExtensions) {
      const auto availableExtensions = possibleDevice.enumerateDeviceExtensionProperties();

      std::set<std::string> requiredExtensions(desiredDeviceExtensions.begin(),
                                               desiredDeviceExtensions.end());

      for (const auto& extension : availableExtensions) {
         requiredExtensions.erase(extension.extensionName);
      }

      return requiredExtensions.empty();
   }

   SwapchainSupportDetails querySwapchainSupport(
       const vk::raii::PhysicalDevice& possibleDevice,
       const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
      SwapchainSupportDetails details;
      details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(**surface);
      details.formats = possibleDevice.getSurfaceFormatsKHR(**surface);
      details.presentModes = possibleDevice.getSurfacePresentModesKHR(**surface);
      return details;
   }

   bool isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice, const Instance& instance) {
      const QueueFamilyIndices queueFamilyIndices =
          findQueueFamilies(possibleDevice, instance.getSurface());

      const bool extensionsSupported =
          checkDeviceExtensionSupport(possibleDevice, instance.getDesiredDeviceExtensions());

      bool swapchainAdequate = false;
      if (extensionsSupported) {
         auto [capabilities, formats, presentModes] =
             querySwapchainSupport(possibleDevice, instance.getSurface());
         swapchainAdequate = !formats.empty() && !presentModes.empty();
      }

      const auto features = possibleDevice.getFeatures();

      const auto props = possibleDevice.getProperties();
      const auto isDiscrete = props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

      return queueFamilyIndices.isComplete() && extensionsSupported && swapchainAdequate &&
             isDiscrete && features.samplerAnisotropy && features.tessellationShader;
   }
}