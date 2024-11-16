#include "Layout.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "Vulkan.hpp"
#include <vulkan/vulkan.hpp>

namespace tr::gfx::sb {
   Layout::Layout(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                  const vk::DescriptorSetLayoutCreateInfo& info,
                  const std::string_view name)
       : vkLayout{std::make_unique<vk::raii::DescriptorSetLayout>(
             newGraphicsDevice->getVulkanDevice()->createDescriptorSetLayout(info))},
         graphicsDevice{std::move(newGraphicsDevice)} {
      Helpers::setObjectName(**vkLayout, *graphicsDevice->getVulkanDevice(), name);
   }

   auto Layout::getVkLayout() const -> const vk::DescriptorSetLayout& {
      return **vkLayout;
   }

   auto Layout::getLayoutSize() const -> vk::DeviceSize {
      return vkLayout->getSizeEXT();
   }

   auto Layout::getAlignedSize() const -> vk::DeviceSize {
      const auto alignment =
          graphicsDevice->getDescriptorBufferProperties().descriptorBufferOffsetAlignment;
      const auto value = getLayoutSize();
      return (value + alignment - 1) & ~(alignment - 1);
   }

   auto Layout::getBindingOffset(const uint32_t binding) const -> vk::DeviceSize {
      return vkLayout->getBindingOffsetEXT(binding);
   }
}