#pragma once
#include <memory>
#include <vector>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

class Instance {
 public:
   explicit Instance(bool validationEnabled = false,
                     uint32_t initialHeight = 1366,
                     uint32_t initialWidth = 768);
   ~Instance();

   std::vector<vk::raii::PhysicalDevice> enumeratePhysicalDevices() const;

   uint32_t getHeight() const {
      return this->height;
   }

   uint32_t getWidth() const {
      return width;
   }

   std::vector<const char*> getDesiredDeviceExtensions() const {
      return desiredDeviceExtensions;
   }

   std::vector<const char*> getDesiredValidationLayers() const {
      return desiredValidationLayers;
   }

   const std::unique_ptr<vk::raii::SurfaceKHR>& getSurface() const {
      return surface;
   }

   bool isValidationEnabled() const {
      return validationEnabled;
   }

   std::unique_ptr<GLFWwindow*> const& getWindow() const {
      return window;
   }

   const std::unique_ptr<vk::raii::Instance>& getVkInstance() const {
      return instance;
   }

   void resizeWindow(uint32_t newHeight, uint32_t newWidth);

 private:
   bool validationEnabled;
   uint32_t height = 0;
   uint32_t width = 0;

   std::vector<const char*> desiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
   std::vector<const char*> desiredValidationLayers = {"VK_LAYER_KHRONOS_validation"};

   std::unique_ptr<GLFWwindow*> window;

   std::unique_ptr<vk::raii::Context> context;
   std::unique_ptr<vk::raii::Instance> instance;
   std::unique_ptr<vk::raii::SurfaceKHR> surface;
   std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
   std::unique_ptr<vk::raii::DebugReportCallbackEXT> reportCallback;

   void createInstance();

   bool checkValidationLayerSupport() const;

   std::pair<std::vector<const char*>, bool> getRequiredExtensions() const;

   static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

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
