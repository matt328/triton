#pragma once

#include "gfx/RenderContextConfig.hpp"

namespace tr {

class DebugPipeline;
class DebugTask;
class VkResourceManager;
class IndirectPipeline;
class StaticPipeline;
class BufferManager;
class IndirectRenderTask;
class ComputeTask;
class StaticTask;
class TerrainTask;

class RenderTaskFactory {
public:
  RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                    std::shared_ptr<IndirectPipeline> newIndirectPipeline,
                    std::shared_ptr<StaticPipeline> newStaticPipeline,
                    std::shared_ptr<DebugPipeline> newDebugPipeline,
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
  auto createDebugTask() -> std::unique_ptr<DebugTask>;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IndirectPipeline> indirectPipeline;
  std::shared_ptr<StaticPipeline> staticPipeline;
  std::shared_ptr<DebugPipeline> debugPipeline;
  std::shared_ptr<BufferManager> bufferManager;
  RenderContextConfig config;
};

}
