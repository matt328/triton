#pragma once

#include <vulkan/vulkan_hpp_macros.hpp>

#include <vulkan/vulkan_core.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

namespace tr::gfx {

   class ImmediateContext;
   class VkContext;

   namespace mem {
      class Allocator;
   }

   namespace Textures {
      class TextureFactory;
   }

   namespace Geometry {
      class MeshFactory;
   }

   class GraphicsDevice {
    public:
      explicit GraphicsDevice(GLFWwindow* window, bool validationEnabled = false);

      ~GraphicsDevice();

      GraphicsDevice(const GraphicsDevice&) = delete;
      GraphicsDevice(GraphicsDevice&&) = delete;
      GraphicsDevice& operator=(const GraphicsDevice&) = delete;
      GraphicsDevice& operator=(GraphicsDevice&&) = delete;

      static constexpr auto DrawImageExtent2D = vk::Extent2D{1920, 1080};

      [[nodiscard]] std::vector<vk::raii::PhysicalDevice> enumeratePhysicalDevices() const;

      [[nodiscard]] std::vector<const char*> getDesiredDeviceExtensions() const {
         return desiredDeviceExtensions;
      }

      [[nodiscard]] std::vector<const char*> getDesiredValidationLayers() const {
         return desiredValidationLayers;
      }

      [[nodiscard]] const std::unique_ptr<vk::raii::SurfaceKHR>& getSurface() const {
         return surface;
      }

      [[nodiscard]] const vk::Instance& getVkInstance() const {
         return **instance;
      }

      [[nodiscard]] vk::Format getSwapchainFormat() const {
         return swapchainImageFormat;
      }

      [[nodiscard]] vk::Extent2D getSwapchainExtent() const {
         return swapchainExtent;
      }

      [[nodiscard]] const vk::raii::PhysicalDevice& getPhysicalDevice() const {
         return *physicalDevice;
      }

      [[nodiscard]] auto getAllocator() const noexcept -> std::shared_ptr<mem::Allocator> {
         return raiillocator;
      }

      /// @brief Returns a const reference to the `vk::raii::Device`. This will only live as long as
      /// the graphics device, so do not store the return value of this function unless you know
      /// what you are doing.
      /// @return The current Vulkan Device.
      [[nodiscard]] const vk::raii::Device& getVulkanDevice() const {
         return *vulkanDevice;
      }

      [[nodiscard]] const vk::raii::CommandPool& getCommandPool() const {
         return *commandPool;
      }

      [[nodiscard]] const vk::raii::Queue& getGraphicsQueue() const {
         return *graphicsQueue;
      }

      [[nodiscard]] const std::vector<vk::raii::ImageView>& getSwapchainImageViews() const {
         return swapchainImageViews;
      }

      [[nodiscard]] const std::vector<vk::Image>& getSwapchainImages() const {
         return swapchainImages;
      }

      [[nodiscard]] const vk::raii::SwapchainKHR& getSwapchain() const {
         return *swapchain;
      }

      [[nodiscard]] auto getAsyncTransferContext() const noexcept -> std::shared_ptr<VkContext> {
         return asyncTransferContext;
      }

      [[nodiscard]] auto getDescriptorBufferProperties() const {
         return descriptorBufferProperties;
      }

      [[nodiscard]] std::pair<uint32_t, uint32_t> getCurrentSize() const;

      void recreateSwapchain();

    private:
      bool validationEnabled;

      static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

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
      std::vector<const char*> desiredValidationLayers = {"VK_LAYER_KHRONOS_validation"};

      std::unique_ptr<vk::raii::Context> context;
      std::unique_ptr<vk::raii::Instance> instance;
      std::unique_ptr<vk::raii::SurfaceKHR> surface;
      std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;
      std::unique_ptr<vk::raii::Device> vulkanDevice;

      vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties;

      std::unique_ptr<vk::raii::SwapchainKHR> oldSwapchain;
      std::unique_ptr<vk::raii::SwapchainKHR> swapchain;
      std::vector<vk::Image> swapchainImages;
      std::vector<vk::raii::ImageView> swapchainImageViews;
      vk::Format swapchainImageFormat = vk::Format::eUndefined;
      vk::Extent2D swapchainExtent;

      std::unique_ptr<vk::raii::CommandPool> commandPool;
      std::shared_ptr<VkContext> asyncTransferContext;

      std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
      std::unique_ptr<vk::raii::DebugReportCallbackEXT> reportCallback;

      std::unique_ptr<vk::raii::Queue> graphicsQueue;
      std::unique_ptr<vk::raii::Queue> presentQueue;
      std::shared_ptr<vk::raii::Queue> transferQueue;
      std::unique_ptr<vk::raii::Queue> computeQueue;

      std::shared_ptr<mem::Allocator> raiillocator;

      void createSwapchain();

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