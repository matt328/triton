#pragma once

namespace tr::gfx::ds {
   class Layout {
    public:
      /*
         TODO: Change this to GraphicsDevice so we can get the descriptor buffer offset
         alignment later
      */
      Layout(const vk::raii::Device& device, const vk::DescriptorSetLayoutCreateInfo& info);
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

      [[nodiscard]] auto getAlignedSize(VkDeviceSize alignment) const {
         const auto value = getLayoutSize();
         return (value + alignment - 1) & ~(alignment - 1);
      }

      [[nodiscard]] auto getBindingOffset(const uint32_t binding) const {
         return vkLayout->getBindingOffsetEXT(binding);
      }

    private:
      std::unique_ptr<vk::raii::DescriptorSetLayout> vkLayout;
   };
}