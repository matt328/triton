#include "RenderTaskFactory.hpp"

namespace tr {

RenderTaskFactory::RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                                     std::shared_ptr<GeometryFactory> newGeometryFactory,
                                     std::shared_ptr<IndirectPipeline> newIndirectPipeline)
    : resourceManager{std::move(newResourceManager)},
      geometryFactory{std::move(newGeometryFactory)},
      indirectPipeline{std::move(newIndirectPipeline)} {
}

auto RenderTaskFactory::createIndirectRenderTask() -> std::unique_ptr<IndirectRenderTask> {
  return std::make_unique<IndirectRenderTask>(resourceManager, geometryFactory, indirectPipeline);
}

auto RenderTaskFactory::createComputeTask() -> std::unique_ptr<ComputeTask> {
  return std::make_unique<ComputeTask>(resourceManager);
}

}
