#pragma once
#include "Instance.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapchainSupportDetails.hpp"

namespace tr::gfx {
   class Surface;

   class PhysicalDevice {
    public:
      explicit PhysicalDevice(const std::shared_ptr<Instance>& instance,
                              const std::shared_ptr<Surface>& surface);
      ~PhysicalDevice();

      PhysicalDevice(const PhysicalDevice&) = delete;
      PhysicalDevice(PhysicalDevice&&) = delete;
      auto operator=(const PhysicalDevice&) -> PhysicalDevice& = delete;
      auto operator=(PhysicalDevice&&) -> PhysicalDevice& = delete;

    private:
      std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;

      std::vector<const char*> desiredDeviceExtensions = {
          VK_KHR_SWAPCHAIN_EXTENSION_NAME,
          VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
          VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
#ifdef __APPLE__
          VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
#endif
          VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
          VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
          //  VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
          VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
          VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
          VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME};
   };

   auto isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                         const vk::raii::SurfaceKHR& surface,
                         const std::vector<const char*>& desiredDeviceExtensions) -> bool;

   auto findQueueFamilies(const vk::raii::PhysicalDevice& possibleDevice,
                          const vk::raii::SurfaceKHR& surface) -> QueueFamilyIndices;

   auto checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                    const std::vector<const char*>& desiredDeviceExtensions)
       -> bool;

   auto querySwapchainSupport(const vk::raii::PhysicalDevice& possibleDevice,
                              const vk::raii::SurfaceKHR& surface) -> SwapchainSupportDetails;
}
