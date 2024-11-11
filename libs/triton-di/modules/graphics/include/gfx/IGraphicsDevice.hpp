#pragma once

namespace tr::gfx {

   namespace mem {
      class Image;
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

      [[nodiscard]] virtual auto getVulkanDevice() const -> std::shared_ptr<vk::raii::Device> = 0;

      [[nodiscard]] virtual auto getSwapchainExtent() -> vk::Extent2D = 0;

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

      [[nodiscard]] virtual auto findDepthFormat() -> vk::Format = 0;
   };
}
