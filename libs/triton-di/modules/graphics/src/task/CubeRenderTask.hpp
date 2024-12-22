#pragma once

#include "IRenderTask.hpp"
#include "cm/Handles.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "gfx/GeometryFactory.hpp"
#include "mem/Buffer.hpp"
#include "pipeline/IndirectPipeline.hpp"

namespace tr {

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

   auto record(vk::raii::CommandBuffer& commandBuffer) -> void override;

 private:
   std::shared_ptr<VkResourceManager> resourceManager;
   std::shared_ptr<GeometryFactory> geometryFactory;
   std::shared_ptr<IndirectPipeline> pipeline;

   MeshHandle meshHandle = -1;

   std::unique_ptr<Buffer> indirectBuffer;
   std::unique_ptr<Buffer> instanceBuffer;

   std::unique_ptr<Buffer> cameraDataBuffer;

   tr::IndirectPushConstants pushConstants;

   vk::Viewport viewport;
   vk::Rect2D snezzor;
};

}
