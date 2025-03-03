#pragma once

#include "gfx/RenderContextConfig.hpp"
#include "pipeline/StaticPipeline.hpp"
#include "task/IRenderTask.hpp"
#include "vk/StaticPushConstants.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class TerrainTask final : public IRenderTask {
public:
  TerrainTask(std::shared_ptr<VkResourceManager> newResourceManager,
              std::shared_ptr<StaticPipeline> newPipeline,
              std::shared_ptr<BufferManager> newBufferManager,
              RenderContextConfig newConfig);

  ~TerrainTask() override = default;

  TerrainTask(const TerrainTask&) = delete;
  TerrainTask(TerrainTask&&) = delete;
  auto operator=(const TerrainTask&) -> TerrainTask& = delete;
  auto operator=(TerrainTask&&) -> TerrainTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame* frame) -> void override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<StaticPipeline> pipeline;
  std::shared_ptr<BufferManager> bufferManager;
  RenderContextConfig config;

  std::string drawImageResourceName;

  StaticPushConstants pushConstants{};
};

}
