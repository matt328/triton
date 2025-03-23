#pragma once

#include "IPipeline.hpp"

namespace tr {

class Device;
class IShaderModuleFactory;
class VkResourceManager;

class TerrainPipeline : public IPipeline {
public:
  TerrainPipeline(const std::shared_ptr<Device>& device,
                  const std::shared_ptr<IShaderModuleFactory>& shaderCompiler,
                  const std::shared_ptr<VkResourceManager>& resourceManager);
  ~TerrainPipeline() override = default;

  TerrainPipeline(const TerrainPipeline&) = delete;
  TerrainPipeline(TerrainPipeline&&) = delete;
  auto operator=(const TerrainPipeline&) -> TerrainPipeline& = delete;
  auto operator=(TerrainPipeline&&) -> TerrainPipeline& = delete;

  [[nodiscard]] auto getPipeline() const -> vk::Pipeline override;
  [[nodiscard]] auto getPipelineLayout() const -> vk::PipelineLayout override;

private:
  std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
  std::unique_ptr<vk::raii::Pipeline> pipeline;
};

}
