#include "RenderTaskFactory.hpp"
#include "gfx/RenderContextConfig.hpp"

namespace tr {

RenderTaskFactory::RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                                     std::shared_ptr<IndirectPipeline> newIndirectPipeline,
                                     RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      indirectPipeline{std::move(newIndirectPipeline)},
      config{newConfig} {
}

auto RenderTaskFactory::createIndirectRenderTask() -> std::unique_ptr<IndirectRenderTask> {
  return std::make_unique<IndirectRenderTask>(resourceManager, indirectPipeline, config);
}

auto RenderTaskFactory::createComputeTask() -> std::unique_ptr<ComputeTask> {
  return std::make_unique<ComputeTask>(resourceManager);
}

}
