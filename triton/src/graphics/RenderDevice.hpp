#pragma once

#include "FrameData.hpp"
#include "Log.hpp"
#include "game/RenderSystem.hpp"
#include "geometry/MeshFactory.hpp"
#include "game/Renderable.hpp"
#include "graphics/renderer/RendererBase.hpp"

class Instance;
class AbstractPipeline;
class TextureFactory;
class Texture;

namespace vma::raii {
   class AllocatedBuffer;
   class Allocator;
}

class ImmediateContext;

class Game;

struct CtxDeleter {
   void operator()(tracy::VkCtx* ctx) {
      TracyVkDestroy(ctx);
   }
};

const auto InitialLookAt =
    glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::zero<glm::vec3>(), glm::vec3(0.f, -1.f, 0.f));

constexpr auto FOV = glm::radians(60.f);
constexpr auto ZNear = 0.1f;
constexpr auto ZFar = 1000.f;

class RenderDevice {
 public:
   void createAllocator(const Instance& instance);

   RenderDevice(const RenderDevice&) = delete;
   RenderDevice(RenderDevice&&) = delete;
   RenderDevice& operator=(const RenderDevice&) = delete;
   RenderDevice& operator=(RenderDevice&&) = delete;
   explicit RenderDevice(const Instance& instance);
   ~RenderDevice();

   void render() {
      drawFrame();
   }

   void waitIdle() const;

   std::string createMesh(const std::string_view& filename);
   uint32_t createTexture(const std::string_view& filename);

   void registerRenderSystem(std::shared_ptr<RenderSystem> renderSystem);

   template <typename T>
   static void setObjectName(T const& handle,
                             const vk::raii::Device& device,
                             const vk::DebugReportObjectTypeEXT objectType,
                             const std::string_view name) {
      // NOLINTNEXTLINE this is just debug anyway
      const auto debugHandle = reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

      const auto debugNameInfo = vk::DebugMarkerObjectNameInfoEXT{
          .objectType = objectType, .object = debugHandle, .pObjectName = name.data()};
      device.debugMarkerSetObjectNameEXT(debugNameInfo);
   }

 private:
   std::string tempTextureId;
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

   std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;

   std::unique_ptr<ImmediateContext> transferImmediateContext;
   std::unique_ptr<ImmediateContext> graphicsImmediateContext;

   std::unique_ptr<vk::raii::RenderPass> renderPass;
   std::unique_ptr<AbstractPipeline> pipeline;

   std::unique_ptr<vma::raii::Allocator> raiillocator;

   std::unique_ptr<vma::raii::AllocatedImage> depthImage;
   std::unique_ptr<vk::raii::ImageView> depthImageView;
   std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

   std::vector<std::unique_ptr<FrameData>> frameData;

   std::unique_ptr<vma::raii::AllocatedBuffer> testBuffer;

   std::unique_ptr<TextureFactory> textureFactory;
   std::unique_ptr<MeshFactory> meshFactory;

   std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
   std::unordered_map<std::string, std::unique_ptr<Mesh<Models::Vertex, uint32_t>>> meshes;

   std::vector<std::unique_ptr<Texture>> textureList = {};

   std::vector<std::unique_ptr<RendererBase>> renderers;
   std::unique_ptr<RendererBase> finishRenderer;

   std::shared_ptr<RenderSystem> renderSystem = nullptr;

   uint32_t currentFrame = 0;
   bool framebufferResized = false;

   // Helpers
   void createPhysicalDevice(const Instance& instance);
   void createLogicalDevice(const Instance& instance);
   void createSwapchain(const Instance& instance);
   void createSwapchainImageViews();
   void createCommandPools(const Instance& instance);
   void createDescriptorPool();

   void createPerFrameData(const vk::raii::DescriptorSetLayout& descriptorSetLayout,
                           const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                           const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout);

   void createDepthResources();
   void createFramebuffers();

   void recreateSwapchain();

   void drawFrame();

   void recordCommandBuffer(FrameData& frameData, unsigned imageIndex) const;

   void updateUniformBuffer(uint32_t currentFrame) const;

   // Utils

   static vk::PresentModeKHR chooseSwapPresentMode(
       const std::vector<vk::PresentModeKHR>& availablePresentModes);

   static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
       const std::vector<vk::SurfaceFormatKHR>& availableFormats);

   [[nodiscard]] vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
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
};
