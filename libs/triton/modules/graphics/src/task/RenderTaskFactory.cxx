#include "RenderTaskFactory.hpp"
#include "TerrainTask.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "task/StaticTask.hpp"

namespace tr {

RenderTaskFactory::RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                                     std::shared_ptr<IndirectPipeline> newIndirectPipeline,
                                     std::shared_ptr<StaticPipeline> newStaticPipeline,
                                     std::shared_ptr<BufferManager> newBufferManager,
                                     RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      indirectPipeline{std::move(newIndirectPipeline)},
      staticPipeline{std::move(newStaticPipeline)},
      bufferManager{std::move(newBufferManager)},
      config{newConfig} {
}

auto RenderTaskFactory::createIndirectRenderTask() -> std::unique_ptr<IndirectRenderTask> {
  return std::make_unique<IndirectRenderTask>(resourceManager,
                                              indirectPipeline,
                                              bufferManager,
                                              config);
}

auto RenderTaskFactory::createComputeTask() -> std::unique_ptr<ComputeTask> {
  return std::make_unique<ComputeTask>(resourceManager);
}

auto RenderTaskFactory::createStaticTask() -> std::unique_ptr<StaticTask> {
  return std::make_unique<StaticTask>(resourceManager, staticPipeline, bufferManager, config);
}

auto RenderTaskFactory::createTerrainTask() -> std::unique_ptr<TerrainTask> {
  return std::make_unique<TerrainTask>(resourceManager, staticPipeline, bufferManager, config);
}

}
