#pragma once

#include "TerrainTask.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "pipeline/IndirectPipeline.hpp"
#include "pipeline/StaticPipeline.hpp"
#include "task/ComputeTask.hpp"
#include "task/IndirectRenderTask.hpp"
#include "task/StaticTask.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class RenderTaskFactory {
public:
  RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                    std::shared_ptr<IndirectPipeline> newIndirectPipeline,
                    std::shared_ptr<StaticPipeline> newStaticPipeline,
                    std::shared_ptr<BufferManager> newBufferManager,
                    RenderContextConfig newConfig);
  ~RenderTaskFactory() = default;

  RenderTaskFactory(RenderTaskFactory&&) = delete;
  RenderTaskFactory(const RenderTaskFactory&) = delete;

  auto operator=(RenderTaskFactory&&) = delete;
  auto operator=(const RenderTaskFactory&) = delete;

  auto createIndirectRenderTask() -> std::unique_ptr<IndirectRenderTask>;
  auto createComputeTask() -> std::unique_ptr<ComputeTask>;
  auto createStaticTask() -> std::unique_ptr<StaticTask>;
  auto createTerrainTask() -> std::unique_ptr<TerrainTask>;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IndirectPipeline> indirectPipeline;
  std::shared_ptr<StaticPipeline> staticPipeline;
  std::shared_ptr<BufferManager> bufferManager;
  RenderContextConfig config;
};

}
