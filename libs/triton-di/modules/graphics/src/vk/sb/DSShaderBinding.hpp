#pragma once

#include "IShaderBinding.hpp"

namespace tr {

class IDebugManager;
class Device;
class DSLayout;

class DSShaderBinding : public IShaderBinding {
public:
  DSShaderBinding(std::shared_ptr<Device> newDevice,
                  const vk::DescriptorPool& pool,
                  vk::DescriptorType descriptorType,
                  const DSLayout& layout,
                  const std::shared_ptr<IDebugManager>& debugManager,
                  std::string_view name = "Unnamed DescriptorSet");
  ~DSShaderBinding() override;

  DSShaderBinding(const DSShaderBinding&) = delete;
  DSShaderBinding(DSShaderBinding&&) = delete;
  auto operator=(const DSShaderBinding&) -> DSShaderBinding& = delete;
  auto operator=(DSShaderBinding&&) -> DSShaderBinding& = delete;

  void bindBuffer(uint32_t binding, const Buffer& buffer, size_t size) override;

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
  std::string name;
  std::shared_ptr<Device> device;
  std::unique_ptr<vk::raii::DescriptorSet> vkDescriptorSet;
  vk::DescriptorType descriptorType;
};

}
