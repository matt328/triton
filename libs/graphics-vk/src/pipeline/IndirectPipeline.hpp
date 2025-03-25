#pragma once

#include "IPipeline.hpp"
#include "VkResourceManager.hpp"

namespace tr {

class IShaderModuleFactory;
class Device;
class VkResourceManager;

class IndirectPipeline : public IPipeline {
public:
  IndirectPipeline(std::shared_ptr<Device> device,
                   std::shared_ptr<IShaderModuleFactory> shaderCompiler,
                   std::shared_ptr<VkResourceManager> resourceManager);
  ~IndirectPipeline() override = default;

  IndirectPipeline(const IndirectPipeline&) = delete;
  IndirectPipeline(IndirectPipeline&&) = delete;
  auto operator=(const IndirectPipeline&) -> IndirectPipeline& = delete;
  auto operator=(IndirectPipeline&&) -> IndirectPipeline& = delete;

  [[nodiscard]] auto getPipeline() const -> vk::Pipeline override;
  [[nodiscard]] auto getPipelineLayout() const -> vk::PipelineLayout override;

private:
  std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
  std::unique_ptr<vk::raii::Pipeline> pipeline;
};

}
