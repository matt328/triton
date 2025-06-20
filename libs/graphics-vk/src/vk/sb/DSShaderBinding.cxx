#include "DSShaderBinding.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "vk/core/Device.hpp"
#include "vk/sb/DSLayout.hpp"

namespace tr {

DSShaderBinding::DSShaderBinding(std::shared_ptr<Device> newDevice,
                                 const vk::DescriptorPool& pool,
                                 const vk::DescriptorType descriptorType,
                                 const DSLayout& layout,
                                 const std::shared_ptr<IDebugManager>& debugManager,
                                 const std::string_view newName)
    : name{newName}, device{std::move(newDevice)}, descriptorType{descriptorType} {
  auto vkLayout = layout.getVkLayout();
  const auto allocInfo = vk::DescriptorSetAllocateInfo{.descriptorPool = pool,
                                                       .descriptorSetCount = 1,
                                                       .pSetLayouts = &vkLayout};

  try {
    vkDescriptorSet.emplace(
        std::move(device->getVkDevice().allocateDescriptorSets(allocInfo).front()));
    debugManager->setObjectName(**vkDescriptorSet, newName);
  } catch (const vk::SystemError& e) { Log.warn("Descriptor Pool is full: {0}", e.what()); }
}

DSShaderBinding::~DSShaderBinding() {
  Log.trace("Destroying ShaderBinding: {0}", name);
}

void DSShaderBinding::bindBuffer(const uint32_t binding,
                                 const ManagedBuffer& buffer,
                                 const size_t size) {
  const auto bufferInfo =
      vk::DescriptorBufferInfo{.buffer = buffer.getVkBuffer(), .offset = 0, .range = size};
  const auto writes = std::array{vk::WriteDescriptorSet{.dstSet = **vkDescriptorSet,
                                                        .dstBinding = binding,
                                                        .dstArrayElement = 0,
                                                        .descriptorCount = 1,
                                                        .descriptorType = descriptorType,
                                                        .pBufferInfo = &bufferInfo}};
  device->getVkDevice().updateDescriptorSets(writes, nullptr);
}

void DSShaderBinding::bindImageSamplers(const uint32_t binding,
                                        const std::vector<vk::DescriptorImageInfo>& imageInfo) {
  ZoneNamedN(a, "Updating Texture DS", true);
  const auto write =
      vk::WriteDescriptorSet{.dstSet = **vkDescriptorSet,
                             .dstBinding = binding,
                             .dstArrayElement = 0,
                             .descriptorCount = static_cast<uint32_t>(imageInfo.size()),
                             .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                             .pImageInfo = imageInfo.data()};
  device->getVkDevice().updateDescriptorSets(write, nullptr);
}

void DSShaderBinding::bindToPipeline(const vk::raii::CommandBuffer& cmd,
                                     const vk::PipelineBindPoint bindPoint,
                                     const uint32_t setIndex,
                                     const vk::PipelineLayout& layout) const {
  cmd.bindDescriptorSets(bindPoint, layout, setIndex, **vkDescriptorSet, {});
}

}
