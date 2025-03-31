#pragma once

#include "gfx/RenderContextConfig.hpp"
#include "task/IRenderTask.hpp"
#include "task/debugshapes/DebugPushConstants.hpp"

namespace tr {

class BufferManager;
class DebugPipeline;
class VkResourceManager;

class DebugTask : IRenderTask {
public:
  explicit DebugTask(std::shared_ptr<BufferManager> newBufferManager,
                     std::shared_ptr<DebugPipeline> newDebugPipeline,
                     std::shared_ptr<VkResourceManager> newResourceManager,
                     const RenderContextConfig& newRenderConfig);
  ~DebugTask() override;

  DebugTask(const DebugTask&) = delete;
  DebugTask(DebugTask&&) = delete;
  auto operator=(const DebugTask&) -> DebugTask& = delete;
  auto operator=(DebugTask&&) -> DebugTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame* frame) -> void override;

  auto registerBuffers(const std::unique_ptr<Frame>& frame) -> void;

private:
  std::shared_ptr<BufferManager> bufferManager;
  std::shared_ptr<DebugPipeline> pipeline;
  std::shared_ptr<VkResourceManager> resourceManager;

  RenderContextConfig renderConfig;

  DebugPushConstants pushConstants{};

  size_t gpuBufferHandleKey{};
  size_t drawCommandBufferKey{};
  size_t objectCountBufferKey{};
  size_t objectDataIndexBufferKey{};
  size_t objectDataBufferKey{};
};

}
