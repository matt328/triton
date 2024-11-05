#include "Layout.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "Vulkan.hpp"

namespace tr::gfx::sb {
   Layout::Layout(std::shared_ptr<IGraphicsDevice> graphicsDevice,
                  const vk::DescriptorSetLayoutCreateInfo& info,
                  const std::string_view name)
       : vkLayout{std::make_unique<vk::raii::DescriptorSetLayout>(
             graphicsDevice->getVulkanDevice().createDescriptorSetLayout(info))},
         graphicsDevice{std::move(graphicsDevice)} {
      Helpers::setObjectName(**vkLayout, graphicsDevice->getVulkanDevice(), name);
   }

   Layout::~Layout() { // NOLINT(*-use-equals-default)
   }
}