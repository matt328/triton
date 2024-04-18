#include "Layout.hpp"

namespace tr::gfx::ds {
   Layout::Layout(const vk::raii::Device& device, const vk::DescriptorSetLayoutCreateInfo& info) {
      vkLayout =
          std::make_unique<vk::raii::DescriptorSetLayout>(device.createDescriptorSetLayout(info));
   }

   Layout::~Layout() {
   }
}