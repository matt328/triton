#include "DSShaderBinding.hpp"
#include "vk/core/Device.hpp"

namespace tr {

/*
  ShaderBindings are going to have to be created from a factory so that they can be of the correct
  implementation based on a renderConfig item (using descriptor buffers or descriptor sets)
  The factory can be container managed, have things injected into it, and itself be injected into
  whichever component owns the ShaderBinding, probably the ResourceManager?

  Consider making them owned by the ResourceManager, and give out opaque handles to them since they
  might be needed in different places, namely the RenderTasks, and we already inject the
  ResourceManager into the RenderTaskFactory

  I think the RenderScheduler should be what tells the ResourceManager what all ShaderBindings it
  will need. ResourceManager will have a handful of different ShaderBindings it knows how to build,
  and it'll just return opaque handles to them, which can then be used to return an IShaderBinding.
*/

DSShaderBinding::DSShaderBinding(std::shared_ptr<Device> newDevice,
                                 const vk::DescriptorPool& pool,
                                 const vk::DescriptorType descriptorType,
                                 const vk::DescriptorSetLayout layout,
                                 const std::string_view newName)
    : name{newName}, device{std::move(newDevice)}, descriptorType{descriptorType} {

  const auto allocInfo = vk::DescriptorSetAllocateInfo{.descriptorPool = pool,
                                                       .descriptorSetCount = 1,
                                                       .pSetLayouts = &layout};

  try {
    vkDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(
        std::move(device->getVkDevice().allocateDescriptorSets(allocInfo).front()));
  } catch (const vk::SystemError& e) { Log.warn("Descriptor Pool is full: {0}", e.what()); }
  Helpers::setObjectName(**vkDescriptorSet, *device, name);
}

DSShaderBinding::~DSShaderBinding() {
  Log.trace("Destroying ShaderBinding: {0}", name);
}

void DSShaderBinding::bindBuffer(const uint32_t binding, const Buffer& buffer, const size_t size) {
  const auto bufferInfo =
      vk::DescriptorBufferInfo{.buffer = buffer.getBuffer(), .offset = 0, .range = size};
  const auto writes = std::array{vk::WriteDescriptorSet{.dstSet = **vkDescriptorSet,
                                                        .dstBinding = binding,
                                                        .dstArrayElement = 0,
                                                        .descriptorCount = 1,
                                                        .descriptorType = descriptorType,
                                                        .pBufferInfo = &bufferInfo}};
  device->updateDescriptorSets(writes, nullptr);
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
  device->updateDescriptorSets(write, nullptr);
}

void DSShaderBinding::bindToPipeline(const vk::raii::CommandBuffer& cmd,
                                     const vk::PipelineBindPoint bindPoint,
                                     const uint32_t setIndex,
                                     const vk::PipelineLayout& layout) const {
  cmd.bindDescriptorSets(bindPoint, layout, setIndex, **vkDescriptorSet, {});
}

}
