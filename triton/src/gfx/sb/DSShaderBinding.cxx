#include "DSShaderBinding.hpp"

namespace tr::gfx::sb {

   DSShaderBinding::DSShaderBinding(const vk::raii::Device& device,
                                    vk::DescriptorType descriptorType)
       : ShaderBinding{}, device{device}, descriptorType{descriptorType} {
   }

   void DSShaderBinding::bindBuffer(const int binding,
                                    const mem::Buffer& buffer,
                                    const size_t size) {
      const auto bufferInfo =
          vk::DescriptorBufferInfo{.buffer = buffer.getBuffer(), .offset = 0, .range = size};
      const auto writes = std::array{vk::WriteDescriptorSet{.dstSet = **vkDescriptorSet,
                                                            .dstBinding = 0,
                                                            .dstArrayElement = 0,
                                                            .descriptorCount = 1,
                                                            .descriptorType = descriptorType,
                                                            .pBufferInfo = &bufferInfo}};
      device.updateDescriptorSets(writes, nullptr);
   }

   void DSShaderBinding::update(){};
}