#pragma once

#include "gfx/IGraphicsDevice.hpp"
#include "tr/IWindow.hpp"

#include "vk/Context.hpp"
#include "vk/Instance.hpp"
#include "IDebugManager.hpp"

#include <vk/Device.hpp>
#include <vk/PhysicalDevice.hpp>

namespace tr::gfx {

   class VkContext;

   namespace geo {
      class ImmutableMesh;
   }

   namespace mem {
      class Allocator;
   }

   namespace tex {
      class Texture;
   }

   class VkGraphicsDevice final : public IGraphicsDevice {
    public:
      struct Config {
         std::vector<const char*> validationLayers;
         bool validationEnabled;
      };

      explicit VkGraphicsDevice(std::shared_ptr<tr::IWindow> newWindow,
                                std::shared_ptr<Context> newContext,
                                std::shared_ptr<IDebugManager> newDebugManager,
                                std::shared_ptr<Instance> newInstance,
                                std::shared_ptr<PhysicalDevice> newPhysicalDevice,
                                std::shared_ptr<Device> newDevice);
      ~VkGraphicsDevice() override;

      VkGraphicsDevice(const VkGraphicsDevice&) = delete;
      VkGraphicsDevice(VkGraphicsDevice&&) = delete;
      auto operator=(const VkGraphicsDevice&) -> VkGraphicsDevice& = delete;
      auto operator=(VkGraphicsDevice&&) -> VkGraphicsDevice& = delete;

      auto getDescriptorBufferProperties()
          -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT override;

      auto submit(const vk::SubmitInfo& submitInfo, const std::unique_ptr<vk::raii::Fence>& fence)
          -> void override;

      auto present(const std::unique_ptr<vk::raii::Semaphore>& semaphore, uint32_t imageIndex)
          -> vk::Result override;

      auto getSwapchainExtent() -> vk::Extent2D override;

      auto getSwapchainFormat() -> vk::Format override {
         return swapchainImageFormat;
      }

      auto getSwapchainImage(uint32_t swapchainImageIndex) -> const vk::Image& override;

      auto getSwapchainImageView(uint32_t swapchainImageIndex)
          -> const vk::raii::ImageView& override;

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
          -> cm::TracyContextPtr override;

      auto uploadVertexData(const geo::GeometryData& geometryData) -> cm::MeshHandle override;

      auto uploadImageData(const as::ImageData& imageData) -> cm::TextureHandle override;

      auto getTextures() const
          -> cm::LockableResource<const std::vector<vk::DescriptorImageInfo>> override;

      auto getMesh(cm::MeshHandle meshHandle) -> geo::ImmutableMesh& override;

      [[nodiscard]] auto findDepthFormat() -> vk::Format override;
      [[nodiscard]] auto acquireNextSwapchainImage(const vk::Semaphore& semaphore)
          -> std::variant<uint32_t, AcquireResult> override;

      void transitionImage(const vk::raii::CommandBuffer& cmd,
                           const vk::Image& image,
                           const vk::ImageLayout& currentLayout,
                           const vk::ImageLayout& newLayout) override;
      void copyImageToImage(const vk::raii::CommandBuffer& cmd,
                            const vk::Image& source,
                            const vk::Image& destination,
                            const vk::Extent2D& srcSize,
                            const vk::Extent2D& dstSize) override;

      auto recreateSwapchain() -> void override;

      auto waitIdle() -> void override;

    private:
      [[nodiscard]] auto getRequiredExtensions() const -> std::pair<std::vector<const char*>, bool>;
      [[nodiscard]] auto enumeratePhysicalDevices() const -> std::vector<vk::raii::PhysicalDevice>;
      [[nodiscard]] auto getCurrentSize() const -> std::pair<uint32_t, uint32_t>;

      void createSwapchain();

      std::shared_ptr<tr::IWindow> window;
      std::shared_ptr<Context> context;
      std::shared_ptr<IDebugManager> debugManager;
      std::shared_ptr<Instance> instance;
      std::shared_ptr<PhysicalDevice> physicalDevice;
      std::shared_ptr<Device> device;

      std::shared_ptr<vk::raii::Queue> graphicsQueue;
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

      vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties;

      std::shared_ptr<VkContext> asyncTransferContext;

      std::vector<geo::ImmutableMesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<tex::Texture>> textureList;
   };
}