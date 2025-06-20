#include "DSShaderBindingFactory.hpp"
#include "gfx/IFrameManager.hpp"
#include "task/Frame.hpp"
#include "vk/core/Device.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/DSShaderBinding.hpp"

namespace tr {

DSShaderBindingFactory::DSShaderBindingFactory(std::shared_ptr<Device> newDevice,
                                               std::shared_ptr<DSLayoutManager> newLayoutManager,
                                               std::shared_ptr<IFrameManager> newFrameManager,
                                               std::shared_ptr<IDebugManager> newDebugManager)
    : device{std::move(newDevice)},
      layoutManager{std::move(newLayoutManager)},
      frameManager{std::move(newFrameManager)},
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
                                                 Handle<DSLayout> layoutHandle)
    -> LogicalHandle<IShaderBinding> {

  assert(type == ShaderBindingType::Textures);

  const auto descriptorType = vk::DescriptorType::eCombinedImageSampler;
  const auto& layout = layoutManager->getLayout(layoutHandle);

  const auto logicalHandle = handleGenerator.requestLogicalHandle();
  for (const auto& frame : frameManager->getFrames()) {
    const auto handle = handleGenerator.requestHandle();
    shaderBindingMap.insert({handle,
                             std::make_unique<DSShaderBinding>(device,
                                                               **permanentPool,
                                                               descriptorType,
                                                               layout,
                                                               debugManager,
                                                               frame->getIndexedName("Texture"))});
    frame->addLogicalShaderBinding(logicalHandle, handle);
  }

  return logicalHandle;
}

auto DSShaderBindingFactory::getShaderBinding(Handle<IShaderBinding> handle) -> IShaderBinding& {
  assert(shaderBindingMap.contains(handle));
  return *shaderBindingMap.at(handle);
}

}
