#pragma once

#include "IPipeline.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class IShaderCompiler;
class Device;
class VkResourceManager;

class IndirectPipeline : public IPipeline {
public:
  IndirectPipeline(const std::shared_ptr<Device>& device,
                   const std::shared_ptr<IShaderCompiler>& shaderCompiler,
                   const std::shared_ptr<VkResourceManager>& resourceManager);
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
