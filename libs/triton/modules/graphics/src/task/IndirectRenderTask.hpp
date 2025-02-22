#pragma once

#include "IRenderTask.hpp"
#include "cm/Handles.hpp"
#include "vk/IndirectPushConstants.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "pipeline/IndirectPipeline.hpp"

namespace tr {

class Frame;
class VkResourceManager;

class IndirectRenderTask final : public IRenderTask {
public:
  IndirectRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                     std::shared_ptr<IndirectPipeline> newPipeline,
                     std::shared_ptr<BufferManager> newBufferManager,
                     RenderContextConfig newConfig);

  ~IndirectRenderTask() override = default;

  IndirectRenderTask(const IndirectRenderTask&) = delete;
  IndirectRenderTask(IndirectRenderTask&&) = delete;
  auto operator=(const IndirectRenderTask&) -> IndirectRenderTask& = delete;
  auto operator=(IndirectRenderTask&&) -> IndirectRenderTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame* frame) -> void override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IndirectPipeline> pipeline;
  std::shared_ptr<BufferManager> bufferManager;
  RenderContextConfig config;

  [[maybe_unused]] MeshHandle meshHandle = -1;

  std::string drawImageResourceName;

  IndirectPushConstants pushConstants{};
};

}
