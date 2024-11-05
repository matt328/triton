#pragma once

#include "ShaderBinding.hpp"

namespace tr::gfx::sb {
   class DSShaderBinding final : public ShaderBinding {
    public:
      DSShaderBinding(std::shared_ptr<vk::raii::Device> newDevice,
                      const vk::DescriptorPool& pool,
                      vk::DescriptorType descriptorType,
                      vk::DescriptorSetLayout layout,
                      std::string_view name = "Unnamed DescriptorSet");

      void bindBuffer(uint32_t binding, const mem::Buffer& buffer, size_t size) override;

      void bindImageSamplers(uint32_t binding,
                             const std::vector<vk::DescriptorImageInfo>& imageInfo) override;
      /// Binds a ShaderBinding to a spot in the pipeline layout.
      /// setIndex refers to the spot in the pipeline layout this descriptor is bound to.
      /// The descriptor's layout must match the layout found at that index in the pipeline layout.
      void bindToPipeline(const vk::raii::CommandBuffer& cmd,
                          vk::PipelineBindPoint bindPoint,
                          uint32_t setIndex,
                          const vk::PipelineLayout& layout) const override;

    private:
      std::shared_ptr<vk::raii::Device> device;
      std::unique_ptr<vk::raii::DescriptorSet> vkDescriptorSet;
      vk::DescriptorType descriptorType;
   };
}