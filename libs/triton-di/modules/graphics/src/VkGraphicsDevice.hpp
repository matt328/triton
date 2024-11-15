#pragma once

#include "geo/Mesh.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "tr/IWindow.hpp"
#include "tex/Texture.hpp"

namespace tr::gfx {

   class VkContext;

   namespace geo {
      class ImmutableMesh;
   }

   namespace mem {
      class Allocator;
   }

   class VkGraphicsDevice : public IGraphicsDevice {
    public:
      struct Config {
         std::vector<const char*> validationLayers;
         bool validationEnabled;
      };

      explicit VkGraphicsDevice(Config config, std::shared_ptr<tr::IWindow> newWindow);
      ~VkGraphicsDevice() override = default;

      VkGraphicsDevice(const VkGraphicsDevice&) = delete;
      VkGraphicsDevice(VkGraphicsDevice&&) = delete;
      auto operator=(const VkGraphicsDevice&) -> VkGraphicsDevice& = delete;
      auto operator=(VkGraphicsDevice&&) -> VkGraphicsDevice& = delete;

      auto getDescriptorBufferProperties()
          -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT override;

      auto getVulkanDevice() const -> std::shared_ptr<vk::raii::Device> override {
         return vulkanDevice;
      }

      auto submit(const vk::SubmitInfo& submitInfo, const std::unique_ptr<vk::raii::Fence>& fence)
          -> void override;

      auto present(const std::unique_ptr<vk::raii::Semaphore>& semaphore, uint32_t imageIndex)
          -> vk::Result override;

      auto getSwapchainExtent() -> vk::Extent2D override;

      auto createPipelineLayout(const vk::PipelineLayoutCreateInfo& createInfo,
                                const std::string& name)
          -> std::unique_ptr<vk::raii::PipelineLayout> override;

      auto createPipeline(const vk::GraphicsPipelineCreateInfo& createInfo, const std::string& name)
          -> std::unique_ptr<vk::raii::Pipeline> override;

      [[nodiscard]] auto createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                     const vma::AllocationCreateInfo& allocationCreateInfo,
                                     const std::string_view& newName) const
          -> std::unique_ptr<mem::Image> override;

      [[nodiscard]] auto createDrawImage(std::string_view newName) const
          -> std::pair<std::unique_ptr<mem::Image>, std::unique_ptr<vk::raii::ImageView>> override;

      auto createStorageBuffer(vk::DeviceSize size, const std::string& name)
          -> std::unique_ptr<mem::Buffer> override;

      auto createUniformBuffer(vk::DeviceSize size, const std::string& name)
          -> std::unique_ptr<mem::Buffer> override;

      auto createCommandBuffer() -> std::unique_ptr<vk::raii::CommandBuffer> override;

      auto createTracyContext(std::string_view name, const vk::raii::CommandBuffer& commandBuffer)
          -> TracyContextPtr override;

      auto uploadVertexData(const geo::GeometryData& geometryData) -> cm::MeshHandle override;

      auto uploadImageData(const as::ImageData& imageData) -> cm::TextureHandle override;

      auto getTextures() const
          -> cm::LockableResource<const std::vector<vk::DescriptorImageInfo>> override;

      auto getMesh(cm::MeshHandle meshHandle) -> geo::ImmutableMesh& override;

      [[nodiscard]] auto findDepthFormat() -> vk::Format override;
      [[nodiscard]] auto acquireNextSwapchainImage(vk::Semaphore semaphore)
          -> std::variant<uint32_t, AcquireResult> override;

    private:
      Config config;
      [[nodiscard]] auto checkValidationLayerSupport() const -> bool;
      [[nodiscard]] auto getRequiredExtensions() const -> std::pair<std::vector<const char*>, bool>;
      [[nodiscard]] auto enumeratePhysicalDevices() const -> std::vector<vk::raii::PhysicalDevice>;
      [[nodiscard]] auto getCurrentSize() const -> std::pair<uint32_t, uint32_t>;

      void createSwapchain();

      template <typename T>
      void setObjectName(T const& handle, const std::string_view name) {
         auto fn = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
             **vulkanDevice,
             "vkDebugMarkerSetObjectNameEXT");

         if (fn != nullptr) {
            // NOLINTNEXTLINE this is just debug anyway
            const auto debugHandle =
                reinterpret_cast<uint64_t>(static_cast<typename T::CType>(handle));

            [[maybe_unused]] const auto debugNameInfo =
                vk::DebugMarkerObjectNameInfoEXT{.objectType = handle.debugReportObjectType,
                                                 .object = debugHandle,
                                                 .pObjectName = name.data()};
            vulkanDevice->debugMarkerSetObjectNameEXT(debugNameInfo);
         }
      }

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

      std::shared_ptr<tr::IWindow> window;

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
      std::shared_ptr<mem::Allocator> allocator;

      std::vector<geo::ImmutableMesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<tex::Texture>> textureList;
   };
}