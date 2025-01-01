#pragma once

#include "task/IRenderTask.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class VkResourceManager;

class ComputeTask : public IRenderTask {
public:
  explicit ComputeTask(std::shared_ptr<VkResourceManager> newResourceManager);
  ~ComputeTask() override = default;

  ComputeTask(ComputeTask&&) = delete;
  ComputeTask(const ComputeTask&) = delete;
  auto operator=(ComputeTask&&) -> ComputeTask& = delete;
  auto operator=(const ComputeTask&) -> ComputeTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;

  PipelineHandle pipelineHandle;
};

}
