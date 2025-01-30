#pragma once

#include "gfx/RenderContextConfig.hpp"
#include "pipeline/StaticPipeline.hpp"
#include "task/IRenderTask.hpp"
#include "vk/StaticPushConstants.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

class StaticTask final : public IRenderTask {
public:
  StaticTask(std::shared_ptr<VkResourceManager> newResourceManager,
             std::shared_ptr<StaticPipeline> newPipeline,
             std::shared_ptr<BufferManager> newBufferManager,
             RenderContextConfig newConfig);

  ~StaticTask() override = default;

  StaticTask(const StaticTask&) = delete;
  StaticTask(StaticTask&&) = delete;
  auto operator=(const StaticTask&) -> StaticTask& = delete;
  auto operator=(StaticTask&&) -> StaticTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<StaticPipeline> pipeline;
  std::shared_ptr<BufferManager> bufferManager;
  RenderContextConfig config;

  std::string drawImageResourceName;

  StaticPushConstants pushConstants{};
};

}
