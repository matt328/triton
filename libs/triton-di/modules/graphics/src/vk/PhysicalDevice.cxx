#include "PhysicalDevice.hpp"

#include "Surface.hpp"

namespace tr::gfx {
   PhysicalDevice::PhysicalDevice(const std::shared_ptr<Instance>& instance,
                                  const std::shared_ptr<Surface>& surface) {
      const auto physicalDevices = instance->getPhysicalDevices();

      if (physicalDevices.empty()) {
         throw std::runtime_error("Failed to find any GPUs with Vulkan Support");
      }

      for (const auto& possibleDevice : physicalDevices) {
         if (isDeviceSuitable(possibleDevice, surface->getVkSurface(), desiredDeviceExtensions)) {
            physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(possibleDevice);
            Log.info("Using physical device: {}",
                     physicalDevice->getProperties().deviceName.data());
            break;
         }
      }
   }

   PhysicalDevice::~PhysicalDevice() {
      Log.trace("Destroying PhysicalDevice");
   }

   auto isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                         const vk::raii::SurfaceKHR& surface,
                         const std::vector<char const*>& desiredDeviceExtensions) -> bool {

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
             isDiscrete && (features.samplerAnisotropy != 0u) && features.tessellationShader;
   }

   auto findQueueFamilies(const vk::raii::PhysicalDevice& possibleDevice,
                          const vk::raii::SurfaceKHR& surface) -> QueueFamilyIndices {
      QueueFamilyIndices queueFamilyIndices;

      const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

      for (int i = 0; const auto& queueFamily : queueFamilies) {
         if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;
            queueFamilyIndices.graphicsFamilyCount = queueFamily.queueCount;
            queueFamilyIndices.graphicsFamilyPriorities.resize(queueFamily.queueCount, 0.f);
            queueFamilyIndices.graphicsFamilyPriorities[0] = 1.f;
         }

         if (possibleDevice.getSurfaceSupportKHR(i, *surface) != 0u) {
            queueFamilyIndices.presentFamily = i;
            queueFamilyIndices.presentFamilyCount = queueFamily.queueCount;
            queueFamilyIndices.presentFamilyPriorities.resize(queueFamily.queueCount, 0.f);
            queueFamilyIndices.presentFamilyPriorities[0] = 1.f;
         }

         if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            queueFamilyIndices.transferFamily = i;
            queueFamilyIndices.transferFamilyCount = queueFamily.queueCount;
            queueFamilyIndices.transferFamilyPriorities.resize(queueFamily.queueCount, 0.f);
            queueFamilyIndices.transferFamilyPriorities[0] = 1.f;
         }

         if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
            queueFamilyIndices.computeFamily = i;
            queueFamilyIndices.computeFamilyCount = queueFamily.queueCount;
            queueFamilyIndices.computeFamilyPriorities.resize(queueFamily.queueCount, 0.f);
            queueFamilyIndices.computeFamilyPriorities[0] = 1.f;
         }

         if (queueFamilyIndices.isComplete()) {
            break;
         }
         i++;
      }
      return queueFamilyIndices;
   }
   auto checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                    const std::vector<char const*>& desiredDeviceExtensions)
       -> bool {
      const auto availableExtensions = possibleDevice.enumerateDeviceExtensionProperties();

      std::set<std::string> requiredExtensions(desiredDeviceExtensions.begin(),
                                               desiredDeviceExtensions.end());

      for (const auto& extension : availableExtensions) {
         requiredExtensions.erase(extension.extensionName);
      }

      return requiredExtensions.empty();
   }
   auto querySwapchainSupport(const vk::raii::PhysicalDevice& possibleDevice,
                              const vk::raii::SurfaceKHR& surface) -> SwapchainSupportDetails {
      SwapchainSupportDetails details;
      details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(*surface);
      details.formats = possibleDevice.getSurfaceFormatsKHR(*surface);
      details.presentModes = possibleDevice.getSurfacePresentModesKHR(*surface);
      return details;
   }
}