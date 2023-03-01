#pragma once
#include "Instance.h"

#include <optional>

namespace vma {
   class Allocator;
}

class RenderDevice {
 public:
   explicit RenderDevice(const std::unique_ptr<Instance>& instance);
   ~RenderDevice();

 private:
   struct QueueFamilyIndices;
   struct SwapchainSupportDetails;

   uint32_t framebufferWidth = 0;
   uint32_t framebufferHeight = 0;

   std::unique_ptr<vk::raii::Device> device;
   std::unique_ptr<vk::raii::Queue> graphicsQueue;
   std::unique_ptr<vk::raii::Queue> presentQueue;
   std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;

   std::unique_ptr<vk::raii::SwapchainKHR> swapchain;
   std::unique_ptr<vk::raii::Semaphore> semaphore;
   std::unique_ptr<vk::raii::Semaphore> renderSemaphore;

   std::vector<vk::Image> swapchainImages;
   std::vector<vk::raii::ImageView> swapchainImageViews;
   vk::Format swapchainImageFormat = vk::Format::eUndefined;
   vk::Extent2D swapchainExtent;

   std::unique_ptr<vk::raii::CommandPool> commandPool;
   std::vector<std::unique_ptr<vk::raii::CommandBuffer>> commandBuffers;

   bool useCompute = false;

   uint32_t computeFamily = 0;
   std::unique_ptr<vk::raii::Queue> computeQueue;

   std::vector<uint32_t> deviceQueueIndices;
   std::vector<std::unique_ptr<vk::raii::Queue>> deviceQueues;

   std::unique_ptr<vk::raii::CommandBuffer> computeCommandBuffer;
   std::unique_ptr<vk::raii::CommandPool> computeCommandPool;

   std::unique_ptr<vma::Allocator> allocator;

   // Helpers
   void createPhysicalDevice(const std::unique_ptr<Instance>& instance);
   void createAllocator(const std::unique_ptr<Instance>& instance);
   void createLogicalDevice(const std::unique_ptr<Instance>& instance);
   void createSwapchain(const std::unique_ptr<Instance>& instance);
   void createSwapchainImageViews();
   void createCommandPools(const std::unique_ptr<Instance>& instance);

   // Utils
   template <typename T>
   static void setObjectName(T const& handle,
                             const std::unique_ptr<vk::raii::Device>& device,
                             const vk::DebugReportObjectTypeEXT objectType,
                             const std::string_view name) {

      const auto debugHandle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

      const auto debugNameInfo = vk::DebugMarkerObjectNameInfoEXT{
          .objectType = objectType, .object = debugHandle, .pObjectName = name.data()};
      device->debugMarkerSetObjectNameEXT(debugNameInfo);
   }

   static vk::PresentModeKHR chooseSwapPresentMode(
       const std::vector<vk::PresentModeKHR>& availablePresentModes);

   static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
       const std::vector<vk::SurfaceFormatKHR>& availableFormats);

   vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                 const std::unique_ptr<Instance>& instance) const;

   static bool isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                                const std::unique_ptr<Instance>& instance);

   static QueueFamilyIndices findQueueFamilies(
       const vk::raii::PhysicalDevice& possibleDevice,
       const std::unique_ptr<vk::raii::SurfaceKHR>& surface);

   static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                           const std::vector<const char*> desiredDeviceExtensions);

   static SwapchainSupportDetails querySwapchainSupport(
       const vk::raii::PhysicalDevice& possibleDevice,
       const std::unique_ptr<vk::raii::SurfaceKHR>& surface);

   // Structs
   struct QueueFamilyIndices {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;
      std::optional<uint32_t> transferFamily;
      std::optional<uint32_t> computeFamily;

      bool isComplete() const {
         return graphicsFamily.has_value() && presentFamily.has_value() &&
                transferFamily.has_value() && computeFamily.has_value();
      }
   };

   struct SwapchainSupportDetails {
      vk::SurfaceCapabilitiesKHR capabilities;
      std::vector<vk::SurfaceFormatKHR> formats;
      std::vector<vk::PresentModeKHR> presentModes;
   };

   struct FrameData {
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore = nullptr;
      std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore = nullptr;
      std::unique_ptr<vk::raii::Fence> inFlightFence = nullptr;

      /*std::unique_ptr<AllocatedBuffer> uniformBuffer = nullptr;*/
      void* uniformBufferData;

      std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout;
      std::unique_ptr<vk::raii::DescriptorSet> descriptorSet;

      void cleanup() {
         // uniformBuffer.reset();
         descriptorSetLayout.reset();
         // descriptorSet.reset();
      }
   };
};
