#pragma once

namespace tr {

class Buffer;

class IShaderBinding {
public:
  IShaderBinding() = default;

  IShaderBinding(const IShaderBinding&) = default;
  auto operator=(const IShaderBinding&) -> IShaderBinding& = default;

  IShaderBinding(IShaderBinding&&) = default;
  auto operator=(IShaderBinding&&) -> IShaderBinding& = default;

  virtual ~IShaderBinding() = default;

  virtual void bindBuffer(uint32_t binding, const Buffer& buffer, size_t size) = 0;

  virtual void bindImageSamplers(uint32_t binding,
                                 const std::vector<vk::DescriptorImageInfo>& imageInfo) = 0;

  virtual void bindToPipeline(const vk::raii::CommandBuffer& cmd,
                              vk::PipelineBindPoint bindPoint,
                              uint32_t setIndex,
                              const vk::PipelineLayout& layout) const = 0;
};

}
