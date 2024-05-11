#pragma once

#include "ShaderBinding.hpp"

namespace tr::gfx::sb {
   class DSShaderBinding : public ShaderBinding {
    public:
      DSShaderBinding(const vk::raii::Device& device,
                      const vk::DescriptorPool& pool,
                      vk::DescriptorType descriptorType,
                      vk::DescriptorSetLayout layout);

      void bindBuffer(const uint32_t binding,
                      const mem::Buffer& buffer,
                      const size_t size) override;

      void bindImageSamplers(const uint32_t binding,
                             const std::vector<vk::DescriptorImageInfo>& imageInfo) override;

      void bindToPipeline(const vk::raii::CommandBuffer& cmd,
                          const vk::PipelineBindPoint bindPoint,
                          const uint32_t setNumber,
                          const vk::PipelineLayout& layout) const override;

    private:
      const vk::raii::Device& device;
      std::unique_ptr<vk::raii::DescriptorSet> vkDescriptorSet;
      vk::DescriptorType descriptorType;
   };
}