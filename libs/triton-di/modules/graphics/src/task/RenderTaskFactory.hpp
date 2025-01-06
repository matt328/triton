#pragma once

#include "pipeline/IndirectPipeline.hpp"
#include "task/ComputeTask.hpp"
#include "task/IndirectRenderTask.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class RenderTaskFactory {
public:
  RenderTaskFactory(std::shared_ptr<VkResourceManager> newResourceManager,
                    std::shared_ptr<IndirectPipeline> newIndirectPipeline);
  ~RenderTaskFactory() = default;

  RenderTaskFactory(RenderTaskFactory&&) = delete;
  RenderTaskFactory(const RenderTaskFactory&) = delete;

  auto operator=(RenderTaskFactory&&) = delete;
  auto operator=(const RenderTaskFactory&) = delete;

  auto createIndirectRenderTask() -> std::unique_ptr<IndirectRenderTask>;
  auto createComputeTask() -> std::unique_ptr<ComputeTask>;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IndirectPipeline> indirectPipeline;
};

}
