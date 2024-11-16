#pragma once

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::sb {
   class Layout {
    public:
      Layout(std::shared_ptr<IGraphicsDevice> graphicsDevice,
             const vk::DescriptorSetLayoutCreateInfo& info,
             std::string_view name = "Unnamed Layout");
      ~Layout() = default;

      Layout(const Layout&) = delete;
      auto operator=(const Layout&) -> Layout& = delete;

      Layout(Layout&&) = delete;
      auto operator=(Layout&&) -> Layout& = delete;

      [[nodiscard]] auto getVkLayout() const -> const vk::DescriptorSetLayout&;

      [[nodiscard]] auto getLayoutSize() const -> vk::DeviceSize;

      [[nodiscard]] auto getAlignedSize() const -> vk::DeviceSize;

      [[nodiscard]] auto getBindingOffset(uint32_t binding) const -> vk::DeviceSize;

    private:
      std::unique_ptr<vk::raii::DescriptorSetLayout> vkLayout;
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
   };
}