#include "Layout.hpp"
#include "gfx/GraphicsDevice.hpp"

namespace tr::gfx::ds {
   Layout::Layout(const GraphicsDevice& device, const vk::DescriptorSetLayoutCreateInfo& info)
       : vkLayout{std::make_unique<vk::raii::DescriptorSetLayout>(
             device.getVulkanDevice().createDescriptorSetLayout(info))},
         graphicsDevice{device} {
   }

   Layout::~Layout() {
   }
}