#pragma once

#include "IRenderTask.hpp"
#include "cm/Handles.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "gfx/GeometryFactory.hpp"
#include "pipeline/IndirectPipeline.hpp"

namespace tr {

class Frame;
class VkResourceManager;

class CubeRenderTask final : public IRenderTask {
public:
  CubeRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                 std::shared_ptr<GeometryFactory> newGeometryFactory,
                 std::shared_ptr<IndirectPipeline> newPipeline);

  ~CubeRenderTask() override = default;

  CubeRenderTask(const CubeRenderTask&) = delete;
  CubeRenderTask(CubeRenderTask&&) = delete;
  auto operator=(const CubeRenderTask&) -> CubeRenderTask& = delete;
  auto operator=(CubeRenderTask&&) -> CubeRenderTask& = delete;

  auto record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void override;

  enum class ResourceSlot : uint8_t {
    IndirectBuffer = 0,
    InstanceBuffer,
    CameraDataBuffer,
    DrawCountBuffer
  };

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<GeometryFactory> geometryFactory;
  std::shared_ptr<IndirectPipeline> pipeline;

  MeshHandle meshHandle = -1;

  std::string drawImageResourceName;

  IndirectPushConstants pushConstants{};
};

}
