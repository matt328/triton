#include "DSShaderBindingFactory.hpp"
#include "vk/core/Device.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/DSShaderBinding.hpp"

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

DSShaderBindingFactory::~DSShaderBindingFactory() {
  Log.trace("Destroying DSShaderBindingFactory");
  shaderBindingMap.clear();
}

auto DSShaderBindingFactory::createShaderBinding(ShaderBindingType type,
                                                 DSLayoutHandle layoutHandle)
    -> ShaderBindingHandle {
  if (type == ShaderBindingType::Textures) {
    const auto descriptorType = vk::DescriptorType::eCombinedImageSampler;
    const auto& layout = layoutManager->getLayout(layoutHandle);

    auto key = keyGen.getKey();

    shaderBindingMap.insert({key,
                             std::make_unique<DSShaderBinding>(device,
                                                               **permanentPool,
                                                               descriptorType,
                                                               layout,
                                                               debugManager,
                                                               "Texture")});
    return key;
  }
  return 1;
}

auto DSShaderBindingFactory::getShaderBinding(ShaderBindingHandle handle) -> IShaderBinding& {
  assert(shaderBindingMap.contains(handle));
  return *shaderBindingMap.at(handle);
}

}
