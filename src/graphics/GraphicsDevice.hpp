#pragma once

#include <vulkan/vulkan_core.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

namespace Triton::Graphics {

   class ImmediateContext;
   class Allocator;

   namespace Textures {
      class TextureFactory;
   }

   namespace Geometry {
      class MeshFactory;
   }

   class GraphicsDevice {
    public:
      explicit GraphicsDevice(GLFWwindow* window, bool validationEnabled = false);

      ~GraphicsDevice() = default;
      GraphicsDevice(const GraphicsDevice&) = delete;
      GraphicsDevice(GraphicsDevice&&) = delete;
      GraphicsDevice& operator=(const GraphicsDevice&) = delete;
      GraphicsDevice& operator=(GraphicsDevice&&) = delete;

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

      [[nodiscard]] const std::unique_ptr<vk::raii::Instance>& getVkInstance() const {
         return instance;
      }

      [[nodiscard]] vk::Format getSwapchainFormat() const {
         return swapchainImageFormat;
      }

      [[nodiscard]] const vk::Extent2D getSwapchainExtent() const {
         return swapchainExtent;
      }

      [[nodiscard]] const vk::raii::PhysicalDevice& getPhysicalDevice() const {
         return *physicalDevice;
      }

      [[nodiscard]] const Allocator& getAllocator() const {
         return *raiillocator;
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

      [[nodiscard]] const vk::raii::DescriptorPool& getDescriptorPool() const {
         return *descriptorPool;
      }

      [[nodiscard]] const vk::raii::Queue& getGraphicsQueue() const {
         return *graphicsQueue;
      }

      [[nodiscard]] const std::vector<vk::raii::ImageView>& getSwapchainImageViews() const {
         return swapchainImageViews;
      }

      [[nodiscard]] const vk::raii::SwapchainKHR& getSwapchain() const {
         return *swapchain;
      }

      void resizeWindow(uint32_t newHeight, uint32_t newWidth);

    private:
      bool validationEnabled;
      int height = 0;
      int width = 0;

      static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

      std::vector<const char*> desiredDeviceExtensions = {
          VK_KHR_SWAPCHAIN_EXTENSION_NAME,
          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
      std::vector<const char*> desiredValidationLayers = {"VK_LAYER_KHRONOS_validation"};

      std::unique_ptr<vk::raii::Context> context;
      std::unique_ptr<vk::raii::Instance> instance;
      std::unique_ptr<vk::raii::SurfaceKHR> surface;
      std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;
      std::unique_ptr<vk::raii::Device> vulkanDevice;

      std::unique_ptr<vk::raii::SwapchainKHR> swapchain;
      std::vector<vk::Image> swapchainImages;
      std::vector<vk::raii::ImageView> swapchainImageViews;
      vk::Format swapchainImageFormat = vk::Format::eUndefined;
      vk::Extent2D swapchainExtent;

      std::unique_ptr<vk::raii::CommandPool> commandPool;
      std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
      std::unique_ptr<ImmediateContext> transferImmediateContext;
      std::unique_ptr<ImmediateContext> graphicsImmediateContext;

      std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
      std::unique_ptr<vk::raii::DebugReportCallbackEXT> reportCallback;

      std::unique_ptr<vk::raii::Queue> graphicsQueue;
      std::unique_ptr<vk::raii::Queue> presentQueue;
      std::shared_ptr<vk::raii::Queue> transferQueue;
      std::unique_ptr<vk::raii::Queue> computeQueue;

      std::unique_ptr<Allocator> raiillocator;
      std::unique_ptr<Textures::TextureFactory> textureFactory;
      std::unique_ptr<Geometry::MeshFactory> meshFactory;

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