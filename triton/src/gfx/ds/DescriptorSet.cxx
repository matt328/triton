#include "DescriptorSet.hpp"

#include "gfx/mem/AllocatedBuffer.hpp"

namespace tr::gfx::ds {
   DescriptorSet::DescriptorSet(const vk::raii::Device& device,
                                const vk::DescriptorSetLayout& layout,
                                const vk::DescriptorPool& pool,
                                const vk::DescriptorType type)
       : device{device}, descriptorType{type} {

      const auto allocInfo = vk::DescriptorSetAllocateInfo{.descriptorPool = pool,
                                                           .descriptorSetCount = 1,
                                                           .pSetLayouts = &layout};

      try {
         descriptorSet = std::make_unique<vk::raii::DescriptorSet>(
             std::move(device.allocateDescriptorSets(allocInfo).front()));
      } catch (const vk::SystemError& e) {
         Log::warn << "Descriptor Pool is full: " << e.what() << std::endl;
      }
   }

   DescriptorSet::~DescriptorSet() {
   }

   void DescriptorSet::writeBuffer(const mem::AllocatedBuffer& buffer, size_t bufferSize) {
      const auto bufferInfo =
          vk::DescriptorBufferInfo{.buffer = buffer.getBuffer(), .offset = 0, .range = bufferSize};

      const auto writes = std::array{vk::WriteDescriptorSet{.dstSet = **descriptorSet,
                                                            .dstBinding = 0,
                                                            .dstArrayElement = 0,
                                                            .descriptorCount = 1,
                                                            .descriptorType = descriptorType,
                                                            .pBufferInfo = &bufferInfo}};
      device.updateDescriptorSets(writes, nullptr);
   }

   const vk::DescriptorSet& DescriptorSet::getVkDescriptorSet() const {
      return **descriptorSet;
   }
}