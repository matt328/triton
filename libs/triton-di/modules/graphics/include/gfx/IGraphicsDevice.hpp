#pragma once

namespace tr::gfx {
   class IGraphicsDevice {
    public:
      IGraphicsDevice() = default;
      virtual ~IGraphicsDevice() = default;

      virtual auto getDescriptorBufferProperties()
          -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT = 0;

      [[nodiscard]] virtual auto getVulkanDevice() const -> std::shared_ptr<vk::raii::Device> = 0;
   };
}
