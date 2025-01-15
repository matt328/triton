#include "DSLayout.hpp"

namespace tr {
DSLayout::DSLayout(std::shared_ptr<Device> newDevice,
                   const std::shared_ptr<IDebugManager>& debugManager,
                   const vk::DescriptorSetLayoutCreateInfo& info,
                   const std::string_view name)
    : device{std::move(newDevice)},
      vkLayout{std::make_unique<vk::raii::DescriptorSetLayout>(
          device->getVkDevice().createDescriptorSetLayout(info))} {

  debugManager->setObjectName(**vkLayout, name);
}

auto DSLayout::getVkLayout() const -> const vk::DescriptorSetLayout* {
  return &**vkLayout;
}

auto DSLayout::getLayoutSize() const -> vk::DeviceSize {
  return vkLayout->getSizeEXT();
}

auto DSLayout::getAlignedSize() const -> vk::DeviceSize {
  const auto alignment = 0;
  // TODO(matt)
  // device->getVkDevice().getDescriptorBufferProperties().descriptorBufferOffsetAlignment;
  const auto value = getLayoutSize();
  return (value + alignment - 1) & ~(alignment - 1);
}

auto DSLayout::getBindingOffset(const uint32_t binding) const -> vk::DeviceSize {
  return vkLayout->getBindingOffsetEXT(binding);
}

}
