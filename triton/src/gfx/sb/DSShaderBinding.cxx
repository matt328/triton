#include "DSShaderBinding.hpp"
#include "gfx/mem/Buffer.hpp"

namespace tr::gfx::sb {

   DSShaderBinding::DSShaderBinding(const vk::raii::Device& device,
                                    vk::DescriptorType descriptorType)
       : ShaderBinding{}, device{device}, descriptorType{descriptorType} {
      // TODO actually create the vkDescriptorSet
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

}