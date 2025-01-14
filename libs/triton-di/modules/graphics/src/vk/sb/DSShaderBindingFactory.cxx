#include "DSShaderBindingFactory.hpp"
#include "vk/core/Device.hpp"
#include "vk/sb/DSLayoutManager.hpp"

namespace tr {

DSShaderBindingFactory::DSShaderBindingFactory(std::shared_ptr<Device> newDevice,
                                               std::shared_ptr<DSLayoutManager> newLayoutManager,
                                               std::shared_ptr<IDebugManager> newDebugManager)
    : device{std::move(newDevice)},
      layoutManager{std::move(newLayoutManager)},
      debugManager{std::move(newDebugManager)} {

  Log.trace("Constructing DSShaderBindingFactory");

  constexpr auto poolSize = std::array{
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 3 * 10},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                             .descriptorCount = 3 * 100},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = 3 * 10},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 3 * 10},
  };

  const vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
               vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT,
      .maxSets = 3 * 10 * static_cast<uint32_t>(poolSize.size()),
      .poolSizeCount = poolSize.size(),
      .pPoolSizes = poolSize.data()};

  permanentPool = std::make_unique<vk::raii::DescriptorPool>(
      device->getVkDevice().createDescriptorPool(poolInfo, nullptr));
}

auto DSShaderBindingFactory::createShaderBinding(ShaderBindingType type) const
    -> ShaderBindingHandle {
  if (type == ShaderBindingType::Textures) {
    const auto descriptorType = vk::DescriptorType::eCombinedImageSampler;
    const auto& layout = layoutManager->getLayout(0);
  }
}

}
