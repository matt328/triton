#pragma once
#include <optional>

#include "FrameData.h"
#include "Instance.h"
#include "Log.h"

namespace vma {
   namespace raii {
      class AllocatedBuffer;
      class Allocator;
   }
}

class ImmediateContext;

class Game;

class RenderDevice {
 public:
   explicit RenderDevice(const Instance& instance);
   ~RenderDevice();

   void render(const Game& game) {
   }

 private:
   struct QueueFamilyIndices;
   struct SwapchainSupportDetails;

   static constexpr uint32_t FRAMES_IN_FLIGHT = 3;
   uint32_t framebufferWidth = 0;
   uint32_t framebufferHeight = 0;

   std::unique_ptr<vk::raii::Device> device;

   std::unique_ptr<vk::raii::Queue> graphicsQueue;
   std::unique_ptr<vk::raii::Queue> presentQueue;
   std::shared_ptr<vk::raii::Queue> transferQueue;
   std::unique_ptr<vk::raii::Queue> computeQueue;

   std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;

   std::unique_ptr<vk::raii::SwapchainKHR> swapchain;

   std::vector<vk::Image> swapchainImages;
   std::vector<vk::raii::ImageView> swapchainImageViews;
   vk::Format swapchainImageFormat = vk::Format::eUndefined;
   vk::Extent2D swapchainExtent;

   std::unique_ptr<vk::raii::CommandPool> commandPool;

   std::unique_ptr<vk::raii::Fence> uploadFence = nullptr;
   std::unique_ptr<vk::raii::CommandPool> uploadCommandPool = nullptr;
   std::unique_ptr<vk::raii::CommandBuffer> uploadCommandBuffer = nullptr;

   std::unique_ptr<vk::raii::CommandBuffer> computeCommandBuffer;
   std::unique_ptr<vk::raii::CommandPool> computeCommandPool;

   std::unique_ptr<ImmediateContext> transferImmediateContext;

   std::unique_ptr<vma::raii::Allocator> raiillocator;

   std::vector<std::unique_ptr<FrameData>> frameData;

   std::unique_ptr<vma::raii::AllocatedBuffer> testBuffer;

   // Helpers
   void createPhysicalDevice(const Instance& instance);
   void createLogicalDevice(const Instance& instance);
   void createSwapchain(const Instance& instance);
   void createSwapchainImageViews();
   void createCommandPools(const Instance& instance);

   void createPerFrameData();

   vk::raii::RenderPass defaultRenderPass() const;
   vk::Format findDepthFormat() const;
   vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates,
                                  vk::ImageTiling tiling,
                                  vk::FormatFeatureFlags features) const;

   // Utils

   template <typename T>
   static void setObjectName(T const& handle,
                             const vk::raii::Device& device,
                             const vk::DebugReportObjectTypeEXT objectType,
                             const std::string_view name) {

      const auto debugHandle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

      const auto debugNameInfo = vk::DebugMarkerObjectNameInfoEXT{
          .objectType = objectType, .object = debugHandle, .pObjectName = name.data()};
      device.debugMarkerSetObjectNameEXT(debugNameInfo);
   }

   static vk::PresentModeKHR chooseSwapPresentMode(
       const std::vector<vk::PresentModeKHR>& availablePresentModes);

   static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
       const std::vector<vk::SurfaceFormatKHR>& availableFormats);

   vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                 const Instance& instance) const;

   static bool isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                                const Instance& instance);

   static QueueFamilyIndices findQueueFamilies(
       const vk::raii::PhysicalDevice& possibleDevice,
       const std::unique_ptr<vk::raii::SurfaceKHR>& surface);

   static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                           std::vector<const char*> desiredDeviceExtensions);

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
};
