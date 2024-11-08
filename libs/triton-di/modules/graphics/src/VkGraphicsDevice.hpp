#pragma once

#include "gfx/IGraphicsDevice.hpp"
#include "tr/IWindow.hpp"

namespace tr::gfx {

   class VkContext;

   namespace mem {
      class Allocator;
   }

   class VkGraphicsDevice : public IGraphicsDevice {
    public:
      struct Config {
         std::vector<const char*> validationLayers;
         bool validationEnabled;
      };

      VkGraphicsDevice(const std::shared_ptr<tr::IWindow>& window, Config config);
      ~VkGraphicsDevice() override = default;

      auto getDescriptorBufferProperties()
          -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT override;

      [[nodiscard]] auto getVulkanDevice() const -> std::shared_ptr<vk::raii::Device> override {
         return vulkanDevice;
      }

    private:
      Config config;
      [[nodiscard]] auto checkValidationLayerSupport() const -> bool;
      [[nodiscard]] auto getRequiredExtensions() const -> std::pair<std::vector<const char*>, bool>;
      [[nodiscard]] auto enumeratePhysicalDevices() const -> std::vector<vk::raii::PhysicalDevice>;
      [[nodiscard]] auto getCurrentSize() const -> std::pair<uint32_t, uint32_t>;

      void createSwapchain();

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

      VKAPI_ATTR static auto VKAPI_CALL
      debugCallbackFn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* pUserData) -> VkBool32;

      static VKAPI_ATTR auto VKAPI_CALL
      vulkanDebugReportCallback(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT objectType,
                                uint64_t object,
                                size_t location,
                                int32_t messageCode,
                                const char* pLayerPrefix,
                                const char* pMessage,
                                void* userData) -> VkBool32;

      std::unique_ptr<vk::raii::Context> context;
      std::unique_ptr<vk::raii::Instance> instance;
      std::unique_ptr<vk::raii::SurfaceKHR> surface;
      std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;
      std::shared_ptr<vk::raii::Device> vulkanDevice;

      std::unique_ptr<vk::raii::Queue> graphicsQueue;
      std::unique_ptr<vk::raii::Queue> presentQueue;
      std::shared_ptr<vk::raii::Queue> transferQueue;
      std::unique_ptr<vk::raii::Queue> computeQueue;

      std::unique_ptr<vk::raii::CommandPool> commandPool;

      std::unique_ptr<vk::raii::SwapchainKHR> oldSwapchain;
      std::unique_ptr<vk::raii::SwapchainKHR> swapchain;
      std::vector<vk::Image> swapchainImages;
      std::vector<vk::raii::ImageView> swapchainImageViews;
      vk::Format swapchainImageFormat = vk::Format::eUndefined;
      vk::Extent2D swapchainExtent;

      std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
      std::unique_ptr<vk::raii::DebugReportCallbackEXT> reportCallback;

      vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties;

      std::shared_ptr<VkContext> asyncTransferContext;
      std::shared_ptr<mem::Allocator> raiillocator;
   };
}