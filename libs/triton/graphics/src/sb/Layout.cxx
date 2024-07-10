#include "Layout.hpp"
#include "GraphicsDevice.hpp"
#include "helpers/Vulkan.hpp"

namespace tr::gfx::ds {
   Layout::Layout(const GraphicsDevice& device,
                  const vk::DescriptorSetLayoutCreateInfo& info,
                  std::string_view name)
       : vkLayout{std::make_unique<vk::raii::DescriptorSetLayout>(
             device.getVulkanDevice().createDescriptorSetLayout(info))},
         graphicsDevice{device} {
      Helpers::setObjectName(**vkLayout, device.getVulkanDevice(), name);
   }

   Layout::~Layout() {
   }
}