#pragma once

#include "gfx/IGraphicsDevice.hpp"

namespace tr::gfx::sb {
   class Layout {
    public:
      Layout(std::shared_ptr<IGraphicsDevice> graphicsDevice,
             const vk::DescriptorSetLayoutCreateInfo& info,
             std::string_view name = "Unnamed Layout");
      ~Layout();

      Layout(const Layout&) = delete;
      auto operator=(const Layout&) -> Layout& = delete;

      Layout(Layout&&) = delete;
      auto operator=(Layout&&) -> Layout& = delete;

      [[nodiscard]] auto getVkLayout() const -> const vk::DescriptorSetLayout& {
         return **vkLayout;
      }

      [[nodiscard]] auto getLayoutSize() const {
         return vkLayout->getSizeEXT();
      }

      [[nodiscard]] auto getAlignedSize() const {
         const auto alignment =
             graphicsDevice->getDescriptorBufferProperties().descriptorBufferOffsetAlignment;
         const auto value = getLayoutSize();
         return (value + alignment - 1) & ~(alignment - 1);
      }

      [[nodiscard]] auto getBindingOffset(const uint32_t binding) const {
         return vkLayout->getBindingOffsetEXT(binding);
      }

    private:
      std::unique_ptr<vk::raii::DescriptorSetLayout> vkLayout;
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
   };
}