#pragma once

#include "pipeline/IPipeline.hpp"

namespace tr {

class Device;
class IShaderModuleFactory;
class VkResourceManager;

class DebugPipeline : public IPipeline {
public:
  DebugPipeline(std::shared_ptr<Device> device,
                std::shared_ptr<IShaderModuleFactory> shaderCompiler,
                std::shared_ptr<VkResourceManager> resourceManager);
  ~DebugPipeline() override = default;

  DebugPipeline(const DebugPipeline&) = delete;
  DebugPipeline(DebugPipeline&&) = delete;
  auto operator=(const DebugPipeline&) -> DebugPipeline& = delete;
  auto operator=(DebugPipeline&&) -> DebugPipeline& = delete;

  [[nodiscard]] auto getPipeline() const -> vk::Pipeline override;
  [[nodiscard]] auto getPipelineLayout() const -> vk::PipelineLayout override;

private:
  std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
  std::unique_ptr<vk::raii::Pipeline> pipeline;
};

}
