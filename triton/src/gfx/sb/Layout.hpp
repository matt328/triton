#pragma once
#include "gfx/GraphicsDevice.hpp"

namespace tr::gfx::ds {
   class Layout {
    public:
      Layout(const GraphicsDevice& device,
             const vk::DescriptorSetLayoutCreateInfo& info,
             std::string_view name = "Unnamed Layout");
      ~Layout();

      Layout(const Layout&) = delete;
      Layout& operator=(const Layout&) = delete;

      Layout(Layout&&) = delete;
      Layout& operator=(Layout&&) = delete;

      [[nodiscard]] const vk::DescriptorSetLayout& getVkLayout() const {
         return **vkLayout;
      }

      [[nodiscard]] auto getLayoutSize() const {
         return vkLayout->getSizeEXT();
      }

      [[nodiscard]] auto getAlignedSize() const {
         const auto alignment =
             graphicsDevice.getDescriptorBufferProperties().descriptorBufferOffsetAlignment;
         const auto value = getLayoutSize();
         return (value + alignment - 1) & ~(alignment - 1);
      }

      [[nodiscard]] auto getBindingOffset(const uint32_t binding) const {
         return vkLayout->getBindingOffsetEXT(binding);
      }

    private:
      std::unique_ptr<vk::raii::DescriptorSetLayout> vkLayout;
      const GraphicsDevice& graphicsDevice;
   };
}