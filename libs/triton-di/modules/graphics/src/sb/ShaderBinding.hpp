#pragma once

namespace tr::gfx::mem {
   class Buffer;
}

namespace tr::gfx::sb {
   class ShaderBinding {
    public:
      ShaderBinding() = default;
      ShaderBinding(const ShaderBinding&) = default;
      ShaderBinding& operator=(const ShaderBinding&) = default;

      ShaderBinding(ShaderBinding&&) = default;
      ShaderBinding& operator=(ShaderBinding&&) = default;
      virtual ~ShaderBinding() = default;

      virtual void bindBuffer(uint32_t binding, const mem::Buffer& buffer, size_t size) = 0;

      virtual void bindImageSamplers(uint32_t binding,
                                     const std::vector<vk::DescriptorImageInfo>& imageInfo) = 0;

      virtual void bindToPipeline(const vk::raii::CommandBuffer& cmd,
                                  vk::PipelineBindPoint bindPoint,
                                  uint32_t setIndex,
                                  const vk::PipelineLayout& layout) const = 0;
   };
}