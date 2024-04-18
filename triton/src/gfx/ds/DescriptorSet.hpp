#pragma once

#include <vulkan/vulkan_enums.hpp>
namespace tr::gfx {
   class AllocatedBuffer;
}

namespace tr::gfx::ds {

   class DescriptorSet {
    public:
      DescriptorSet(const vk::raii::Device& device,
                    const vk::DescriptorSetLayout& layout,
                    const vk::DescriptorPool& pool,
                    const vk::DescriptorType type);
      ~DescriptorSet();

      DescriptorSet(const DescriptorSet&) = delete;
      DescriptorSet& operator=(const DescriptorSet&) = delete;

      DescriptorSet(DescriptorSet&&) = delete;
      DescriptorSet& operator=(DescriptorSet&&) = delete;

      void writeBuffer(const AllocatedBuffer& buffer, size_t bufferSize);

      [[nodiscard]] const vk::DescriptorSet& getVkDescriptorSet() const;

    private:
      const vk::raii::Device& device;
      std::unique_ptr<vk::raii::DescriptorSet> descriptorSet;
      vk::DescriptorType descriptorType;
   };
}