#pragma once

#include <vulkan/vulkan_enums.hpp>

namespace Triton::Graphics::Helpers {

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

   inline vk::PresentModeKHR chooseSwapPresentMode(
       const std::vector<vk::PresentModeKHR>& availablePresentModes) {
      for (const auto& availablePresentMode : availablePresentModes) {
         if (availablePresentMode == vk::PresentModeKHR::eFifo) {
            return availablePresentMode;
         }
      }
      return vk::PresentModeKHR::eFifo;
   }

   inline vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
       const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
             availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }

   inline vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                        const std::pair<uint32_t, uint32_t>& windowSize) {
      if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
         return capabilities.currentExtent;
      } else {
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

   inline QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& possibleDevice,
                                               const vk::raii::SurfaceKHR& surface) {
      QueueFamilyIndices queueFamilyIndices;

      const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

      for (int i = 0; const auto& queueFamily : queueFamilies) {
         if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;
         }

         if (possibleDevice.getSurfaceSupportKHR(i, *surface)) {
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

   inline bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                           const std::vector<const char*> desiredDeviceExtensions) {
      const auto availableExtensions = possibleDevice.enumerateDeviceExtensionProperties();

      std::set<std::string> requiredExtensions(desiredDeviceExtensions.begin(),
                                               desiredDeviceExtensions.end());

      for (const auto& extension : availableExtensions) {
         requiredExtensions.erase(extension.extensionName);
      }

      return requiredExtensions.empty();
   }

   inline SwapchainSupportDetails querySwapchainSupport(
       const vk::raii::PhysicalDevice& possibleDevice,
       const vk::raii::SurfaceKHR& surface) {
      SwapchainSupportDetails details;
      details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(*surface);
      details.formats = possibleDevice.getSurfaceFormatsKHR(*surface);
      details.presentModes = possibleDevice.getSurfacePresentModesKHR(*surface);
      return details;
   }

   inline bool isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                                const vk::raii::SurfaceKHR& surface,
                                const std::vector<const char*>& desiredDeviceExtensions) {
      const QueueFamilyIndices queueFamilyIndices = findQueueFamilies(possibleDevice, surface);

      const bool extensionsSupported =
          checkDeviceExtensionSupport(possibleDevice, desiredDeviceExtensions);

      bool swapchainAdequate = false;
      if (extensionsSupported) {
         auto [capabilities, formats, presentModes] =
             querySwapchainSupport(possibleDevice, surface);
         swapchainAdequate = !formats.empty() && !presentModes.empty();
      }

      const auto features = possibleDevice.getFeatures();

      const auto props = possibleDevice.getProperties();
      const auto isDiscrete = props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

      return queueFamilyIndices.isComplete() && extensionsSupported && swapchainAdequate &&
             isDiscrete && features.samplerAnisotropy && features.tessellationShader;
   }

   template <typename T>
   inline void setObjectName(T const& handle,
                             [[maybe_unused]] const vk::raii::Device& device,
                             const vk::DebugReportObjectTypeEXT objectType,
                             const std::string_view name) {
      // NOLINTNEXTLINE this is just debug anyway
      const auto debugHandle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

      [[maybe_unused]] const auto debugNameInfo =
          vk::DebugMarkerObjectNameInfoEXT{.objectType = objectType,
                                           .object = debugHandle,
                                           .pObjectName = name.data()};
      device.debugMarkerSetObjectNameEXT(debugNameInfo);
   }

   inline vk::Format findSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice,
                                         const std::vector<vk::Format>& candidates,
                                         const vk::ImageTiling tiling,
                                         const vk::FormatFeatureFlags features) {
      for (const auto format : candidates) {
         auto props = physicalDevice.getFormatProperties(format);
         if ((tiling == vk::ImageTiling::eLinear &&
              (props.linearTilingFeatures & features) == features) ||
             (tiling == vk::ImageTiling::eOptimal &&
              (props.optimalTilingFeatures & features) == features)) {
            return format;
         }
      }
      throw std::runtime_error("Failed to find supported format");
   }

   inline vk::Format findDepthFormat(const vk::raii::PhysicalDevice& physicalDevice) {
      return findSupportedFormat(
          physicalDevice,
          {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
          vk::ImageTiling::eOptimal,
          vk::FormatFeatureFlagBits::eDepthStencilAttachment);
   }

}