#pragma once

#include "gfx/GeometryFactory.hpp"
#include "pipeline/IndirectPipeline.hpp"
#include "task/CubeRenderTask.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class RenderTaskFactory {
public:
  RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                    std::shared_ptr<GeometryFactory> newGeometryFactory,
                    std::shared_ptr<IndirectPipeline> newIndirectPipeline);
  ~RenderTaskFactory() = default;

  RenderTaskFactory(RenderTaskFactory&&) = delete;
  RenderTaskFactory(const RenderTaskFactory&) = delete;

  auto operator=(RenderTaskFactory&&) = delete;
  auto operator=(const RenderTaskFactory&) = delete;

  auto createCubeRenderTask() -> std::unique_ptr<CubeRenderTask>;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<GeometryFactory> geometryFactory;
  std::shared_ptr<IndirectPipeline> indirectPipeline;
};

}
