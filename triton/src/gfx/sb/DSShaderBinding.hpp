#pragma once

#include "ShaderBinding.hpp"

namespace tr::gfx::sb {
   class DSShaderBinding : public ShaderBinding {
    public:
      DSShaderBinding(const vk::raii::Device& device,
                      const vk::DescriptorPool& pool,
                      vk::DescriptorType descriptorType,
                      vk::DescriptorSetLayout layout,
                      std::string_view name = "Unnamed DescriptorSet");

      void bindBuffer(const uint32_t binding,
                      const mem::Buffer& buffer,
                      const size_t size) override;

      void bindImageSamplers(const uint32_t binding,
                             const std::vector<vk::DescriptorImageInfo>& imageInfo) override;
      /// Binds a ShaderBinding to a spot in the pipeline layout.
      /// setIndex refers to the spot in the pipeline layout this descriptor is bound to.
      /// The descriptor's layout must match the layout found at that index in the pipeline layout.
      void bindToPipeline(const vk::raii::CommandBuffer& cmd,
                          const vk::PipelineBindPoint bindPoint,
                          const uint32_t setIndex,
                          const vk::PipelineLayout& layout) const override;

    private:
      // Note to Self: A reference is the appropriate choice here since it conveys the fact that the
      // Device is intended to outlive the DSShaderBinding. The linter doesn't like it, but in this
      // game architecture it is the right choice.
      const vk::raii::Device& device;
      std::unique_ptr<vk::raii::DescriptorSet> vkDescriptorSet;
      vk::DescriptorType descriptorType;
   };
}