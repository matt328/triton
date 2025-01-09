#pragma once

#include "IRenderTask.hpp"
#include "cm/Handles.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "pipeline/IndirectPipeline.hpp"

namespace tr {

class Frame;
class VkResourceManager;

class IndirectRenderTask final : public IRenderTask {
public:
  IndirectRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                     std::shared_ptr<IndirectPipeline> newPipeline,
                     RenderContextConfig newConfig);

  ~IndirectRenderTask() override = default;

  IndirectRenderTask(const IndirectRenderTask&) = delete;
  IndirectRenderTask(IndirectRenderTask&&) = delete;
  auto operator=(const IndirectRenderTask&) -> IndirectRenderTask& = delete;
  auto operator=(IndirectRenderTask&&) -> IndirectRenderTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void override;

  enum class ResourceSlot : uint8_t {
    IndirectBuffer = 0,
    InstanceBuffer,
    CameraDataBuffer,
    DrawCountBuffer
  };

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IndirectPipeline> pipeline;
  RenderContextConfig config;

  [[maybe_unused]] MeshHandle meshHandle = -1;

  std::string drawImageResourceName;

  IndirectPushConstants pushConstants{};
};

}
