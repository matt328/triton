#pragma once

#include "pipeline/IPipeline.hpp"

namespace tr {

class Device;
class IShaderCompiler;

class ComputePipeline : public IPipeline {

public:
  ComputePipeline(const std::shared_ptr<Device>& device,
                  const std::shared_ptr<IShaderCompiler>& shaderCompiler);
  ~ComputePipeline() override = default;

  ComputePipeline(const ComputePipeline&) = delete;
  ComputePipeline(ComputePipeline&&) = delete;
  auto operator=(const ComputePipeline&) -> ComputePipeline& = delete;
  auto operator=(ComputePipeline&&) -> ComputePipeline& = delete;

  [[nodiscard]] auto getPipeline() const -> vk::Pipeline override;
  [[nodiscard]] auto getPipelineLayout() const -> vk::PipelineLayout override;

private:
  std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
  std::unique_ptr<vk::raii::Pipeline> pipeline;
};

}
