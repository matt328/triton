#include "RenderTaskFactory.hpp"

namespace tr {

RenderTaskFactory::RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                                     std::shared_ptr<GeometryFactory> newGeometryFactory,
                                     std::shared_ptr<IndirectPipeline> newIndirectPipeline)
    : resourceManager{std::move(newResourceManager)},
      geometryFactory{std::move(newGeometryFactory)},
      indirectPipeline{std::move(newIndirectPipeline)} {
}

auto RenderTaskFactory::createCubeRenderTask() -> std::unique_ptr<CubeRenderTask> {
  return std::make_unique<CubeRenderTask>(resourceManager, geometryFactory, indirectPipeline);
}

auto RenderTaskFactory::createComputeTask() -> std::unique_ptr<ComputeTask> {
  return std::make_unique<ComputeTask>(resourceManager);
}

}
