#include "DSShaderBinding.hpp"
#include "helpers/Vulkan.hpp"
#include "mem/Buffer.hpp"

namespace tr::gfx::sb {

   DSShaderBinding::DSShaderBinding(const vk::raii::Device& device,
                                    const vk::DescriptorPool& pool,
                                    vk::DescriptorType descriptorType,
                                    vk::DescriptorSetLayout layout,
                                    std::string_view name)
       : ShaderBinding{}, device{device}, descriptorType{descriptorType} {

      const auto allocInfo = vk::DescriptorSetAllocateInfo{.descriptorPool = pool,
                                                           .descriptorSetCount = 1,
                                                           .pSetLayouts = &layout};

      try {
         vkDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(
             std::move(device.allocateDescriptorSets(allocInfo).front()));
      } catch (const vk::SystemError& e) { Log.warn("Descriptor Pool is full: {0}", e.what()); }
      Helpers::setObjectName(**vkDescriptorSet, device, name);
   }

   void DSShaderBinding::bindBuffer(const uint32_t binding,
                                    const mem::Buffer& buffer,
                                    const size_t size) {
      const auto bufferInfo =
          vk::DescriptorBufferInfo{.buffer = buffer.getBuffer(), .offset = 0, .range = size};
      const auto writes = std::array{vk::WriteDescriptorSet{.dstSet = **vkDescriptorSet,
                                                            .dstBinding = binding,
                                                            .dstArrayElement = 0,
                                                            .descriptorCount = 1,
                                                            .descriptorType = descriptorType,
                                                            .pBufferInfo = &bufferInfo}};
      device.updateDescriptorSets(writes, nullptr);
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
      device.updateDescriptorSets(write, nullptr);
   }

   void DSShaderBinding::bindToPipeline(const vk::raii::CommandBuffer& cmd,
                                        const vk::PipelineBindPoint bindPoint,
                                        const uint32_t setIndex,
                                        const vk::PipelineLayout& layout) const {
      cmd.bindDescriptorSets(bindPoint, layout, setIndex, **vkDescriptorSet, nullptr);
   }

}