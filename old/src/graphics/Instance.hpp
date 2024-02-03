#pragma once

#include <vulkan/vulkan_core.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

namespace Triton {

   class Device {
    public:
      explicit Device(GLFWwindow* window, bool validationEnabled = false);

      ~Device() = default;
      Device(const Device&) = delete;
      Device(Device&&) = delete;
      Device& operator=(const Device&) = delete;
      Device& operator=(Device&&) = delete;

      [[nodiscard]] std::vector<vk::raii::PhysicalDevice> enumeratePhysicalDevices() const;

      [[nodiscard]] std::pair<uint32_t, uint32_t> getWindowSize() const {
         return std::make_pair(height, width);
      }

      [[nodiscard]] std::vector<const char*> getDesiredDeviceExtensions() const {
         return desiredDeviceExtensions;
      }

      [[nodiscard]] std::vector<const char*> getDesiredValidationLayers() const {
         return desiredValidationLayers;
      }

      [[nodiscard]] const std::unique_ptr<vk::raii::SurfaceKHR>& getSurface() const {
         return surface;
      }

      [[nodiscard]] bool isValidationEnabled() const {
         return validationEnabled;
      }

      [[nodiscard]] const std::unique_ptr<vk::raii::Instance>& getVkInstance() const {
         return instance;
      }

      void resizeWindow(uint32_t newHeight, uint32_t newWidth);

    private:
      bool validationEnabled;
      int height = 0;
      int width = 0;

      std::vector<const char*> desiredDeviceExtensions = {
          VK_KHR_SWAPCHAIN_EXTENSION_NAME,
          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
      std::vector<const char*> desiredValidationLayers = {"VK_LAYER_KHRONOS_validation"};

      std::unique_ptr<vk::raii::Context> context;
      std::unique_ptr<vk::raii::Instance> instance;
      std::unique_ptr<vk::raii::SurfaceKHR> surface;
      std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
      std::unique_ptr<vk::raii::DebugReportCallbackEXT> reportCallback;

      [[nodiscard]] bool checkValidationLayerSupport() const;

      [[nodiscard]] std::pair<std::vector<const char*>, bool> getRequiredExtensions() const;

      VKAPI_ATTR static VkBool32 VKAPI_CALL
      debugCallbackFn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* pUserData);

      static VKAPI_ATTR VkBool32 VKAPI_CALL
      vulkanDebugReportCallback(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT objectType,
                                uint64_t object,
                                size_t location,
                                int32_t messageCode,
                                const char* pLayerPrefix,
                                const char* pMessage,
                                void* userData);
   };
}