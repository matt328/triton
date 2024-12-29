#pragma once

#include "IPipeline.hpp"
#include "vk/Device.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace tr {
class IShaderCompiler;

class IndirectPipeline : public IPipeline {
public:
  IndirectPipeline(const std::shared_ptr<Device>& device,
                   const std::shared_ptr<IShaderCompiler>& shaderCompiler);
  ~IndirectPipeline() override = default;

  IndirectPipeline(const IndirectPipeline&) = delete;
  IndirectPipeline(IndirectPipeline&&) = delete;
  auto operator=(const IndirectPipeline&) -> IndirectPipeline& = delete;
  auto operator=(IndirectPipeline&&) -> IndirectPipeline& = delete;

  void bind(const vk::raii::CommandBuffer& cmd) override;
  void applyShaderBinding(const ShaderBinding& binding,
                          uint32_t setIndex,
                          const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) override;

  [[nodiscard]] auto getPipeline() -> vk::Pipeline;
  [[nodiscard]] auto getPipelineLayout() -> vk::PipelineLayout;

private:
  std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
  std::unique_ptr<vk::raii::Pipeline> pipeline;
};
}
