#pragma once

#include "IPipeline.hpp"

namespace tr {

class Device;
class IShaderCompiler;
class VkResourceManager;

class StaticPipeline : public IPipeline {
public:
  StaticPipeline(const std::shared_ptr<Device>& device,
                 const std::shared_ptr<IShaderCompiler>& shaderCompiler,
                 const std::shared_ptr<VkResourceManager>& resourceManager);
  ~StaticPipeline() override = default;

  StaticPipeline(const StaticPipeline&) = delete;
  StaticPipeline(StaticPipeline&&) = delete;
  auto operator=(const StaticPipeline&) -> StaticPipeline& = delete;
  auto operator=(StaticPipeline&&) -> StaticPipeline& = delete;

  [[nodiscard]] auto getPipeline() const -> vk::Pipeline override;
  [[nodiscard]] auto getPipelineLayout() const -> vk::PipelineLayout override;

private:
  std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
  std::unique_ptr<vk::raii::Pipeline> pipeline;
};

}
