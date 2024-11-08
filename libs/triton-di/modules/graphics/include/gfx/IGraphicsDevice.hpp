#pragma once

namespace tr::gfx {
   class IGraphicsDevice {
    public:
      IGraphicsDevice() = default;
      virtual ~IGraphicsDevice() = default;

      IGraphicsDevice(const IGraphicsDevice&) = default;
      IGraphicsDevice(IGraphicsDevice&&) = delete;
      auto operator=(const IGraphicsDevice&) -> IGraphicsDevice& = default;
      auto operator=(IGraphicsDevice&&) -> IGraphicsDevice& = delete;

      virtual auto getDescriptorBufferProperties()
          -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT = 0;

      [[nodiscard]] virtual auto getVulkanDevice() const -> std::shared_ptr<vk::raii::Device> = 0;
   };
}
