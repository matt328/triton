#include "Layout.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "Vulkan.hpp"

namespace tr::gfx::sb {
   Layout::Layout(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                  const vk::DescriptorSetLayoutCreateInfo& info,
                  const std::string_view name)
       : vkLayout{std::make_unique<vk::raii::DescriptorSetLayout>(
             newGraphicsDevice->getVulkanDevice()->createDescriptorSetLayout(info))},
         graphicsDevice{std::move(newGraphicsDevice)} {
      Helpers::setObjectName(**vkLayout, *graphicsDevice->getVulkanDevice(), name);
   }

   Layout::~Layout() { // NOLINT(*-use-equals-default)
   }
}