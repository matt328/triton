#pragma once

#include "cm/Handles.hpp"
#include "cm/LockableResource.hpp"
#include "cm/TracyDefs.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace tr::as {
   class ImageData;
}

namespace tr::gfx {

   namespace mem {
      class Image;
      class Buffer;
   }

   enum class AcquireResult {
      Success,
      NeedsResize,
      Error
   };

   namespace geo {
      class GeometryData;
      class ImmutableMesh;
   }

   class IGraphicsDevice {
    public:
      IGraphicsDevice() = default;
      virtual ~IGraphicsDevice() = default;

      IGraphicsDevice(const IGraphicsDevice&) = default;
      IGraphicsDevice(IGraphicsDevice&&) = delete;
      auto operator=(const IGraphicsDevice&) -> IGraphicsDevice& = default;
      auto operator=(IGraphicsDevice&&) -> IGraphicsDevice& = delete;

      static constexpr auto DrawImageExtent2D = vk::Extent2D{1920, 1080};

      virtual auto getDescriptorBufferProperties()
          -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT = 0;

      virtual auto recreateSwapchain() -> void = 0;

      // Escape Hatches
      [[nodiscard]] virtual auto getVulkanDevice() const -> std::shared_ptr<vk::raii::Device> = 0;
      [[nodiscard]] virtual auto getVulkanInstance() const
          -> std::shared_ptr<vk::raii::Instance> = 0;
      [[nodiscard]] virtual auto getGraphicsQueue() const -> std::shared_ptr<vk::raii::Queue> = 0;
      [[nodiscard]] virtual auto getPhysicalDevice() const
          -> std::shared_ptr<vk::raii::PhysicalDevice> = 0;

      virtual auto submit(const vk::SubmitInfo& submitInfo,
                          const std::unique_ptr<vk::raii::Fence>& fence) -> void = 0;

      [[nodiscard]] virtual auto present(const std::unique_ptr<vk::raii::Semaphore>& semaphore,
                                         uint32_t imageIndex) -> vk::Result = 0;

      [[nodiscard]] virtual auto getSwapchainExtent() -> vk::Extent2D = 0;
      [[nodiscard]] virtual auto getSwapchainFormat() -> vk::Format = 0;
      [[nodiscard]] virtual auto getSwapchainImage(uint32_t swapchainImageIndex)
          -> const vk::Image& = 0;
      [[nodiscard]] virtual auto getSwapchainImageView(uint32_t swapchainImageIndex)
          -> const vk::raii::ImageView& = 0;

      [[nodiscard]] virtual auto createPipelineLayout(
          const vk::PipelineLayoutCreateInfo& createInfo,
          const std::string& name) -> std::unique_ptr<vk::raii::PipelineLayout> = 0;

      [[nodiscard]] virtual auto createPipeline(const vk::GraphicsPipelineCreateInfo& createInfo,
                                                const std::string& name)
          -> std::unique_ptr<vk::raii::Pipeline> = 0;

      [[nodiscard]] virtual auto createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                             const vma::AllocationCreateInfo& allocationCreateInfo,
                                             const std::string_view& newName) const
          -> std::unique_ptr<mem::Image> = 0;

      [[nodiscard]] virtual auto createDrawImage(std::string_view newName) const
          -> std::pair<std::unique_ptr<mem::Image>, std::unique_ptr<vk::raii::ImageView>> = 0;

      [[nodiscard]] virtual auto createStorageBuffer(vk::DeviceSize size, const std::string& name)
          -> std::unique_ptr<mem::Buffer> = 0;

      [[nodiscard]] virtual auto createUniformBuffer(vk::DeviceSize size, const std::string& name)
          -> std::unique_ptr<mem::Buffer> = 0;

      [[nodiscard]] virtual auto createCommandBuffer()
          -> std::unique_ptr<vk::raii::CommandBuffer> = 0;

      [[nodiscard]] virtual auto createTracyContext(std::string_view name,
                                                    const vk::raii::CommandBuffer& commandBuffer)
          -> cm::TracyContextPtr = 0;

      [[nodiscard]] virtual auto uploadVertexData(const geo::GeometryData& geometryData)
          -> cm::MeshHandle = 0;

      [[nodiscard]] virtual auto uploadImageData(const as::ImageData& imageData)
          -> cm::TextureHandle = 0;

      [[nodiscard]] virtual auto findDepthFormat() -> vk::Format = 0;

      [[nodiscard]] virtual auto acquireNextSwapchainImage(const vk::Semaphore& semaphore)
          -> std::variant<uint32_t, AcquireResult> = 0;

      virtual auto waitIdle() -> void = 0;

      [[nodiscard]] virtual auto getTextures() const
          -> cm::LockableResource<const std::vector<vk::DescriptorImageInfo>> = 0;

      [[nodiscard]] virtual auto getMesh(cm::MeshHandle meshHandle) -> geo::ImmutableMesh& = 0;

      virtual void transitionImage(const vk::raii::CommandBuffer& cmd,
                                   const vk::Image& image,
                                   const vk::ImageLayout& currentLayout,
                                   const vk::ImageLayout& newLayout) = 0;

      virtual void copyImageToImage(const vk::raii::CommandBuffer& cmd,
                                    const vk::Image& source,
                                    const vk::Image& destination,
                                    const vk::Extent2D& srcSize,
                                    const vk::Extent2D& dstSize) = 0;
   };
}
