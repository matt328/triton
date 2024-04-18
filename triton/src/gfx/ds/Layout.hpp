#pragma once

namespace tr::gfx::ds {
   class Layout {
    public:
      Layout(const vk::raii::Device& device, const vk::DescriptorSetLayoutCreateInfo& info);
      ~Layout();

      Layout(const Layout&) = delete;
      Layout& operator=(const Layout&) = delete;

      Layout(Layout&&) = delete;
      Layout& operator=(Layout&&) = delete;

      [[nodiscard]] const vk::DescriptorSetLayout& getVkLayout() const {
         return **vkLayout;
      }

    private:
      std::unique_ptr<vk::raii::DescriptorSetLayout> vkLayout;
   };
}