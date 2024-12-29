#include "CubeRenderTask.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "cm/ObjectData.hpp"
#include "task/Frame.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

CubeRenderTask::CubeRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                               std::shared_ptr<GeometryFactory> newGeometryFactory,
                               std::shared_ptr<IndirectPipeline> newPipeline)
    : resourceManager{std::move(newResourceManager)},
      geometryFactory{std::move(newGeometryFactory)},
      pipeline{std::move(newPipeline)} {

  const auto geometryHandle = geometryFactory->createUnitCube();
  meshHandle = resourceManager->asyncUpload(geometryFactory->getGeometryData(geometryHandle));
}

auto CubeRenderTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void {

  auto& instanceBuffer = resourceManager->getBuffer(frame.getIndexedName("InstanceDataBuffer"));
  auto& cameraDataBuffer = resourceManager->getBuffer(frame.getIndexedName("CameraDataBuffer"));

  pushConstants = IndirectPushConstants{.drawID = 0,
                                        .baseAddress = instanceBuffer.getDeviceAddress(),
                                        .cameraDataAddress = cameraDataBuffer.getDeviceAddress()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  const auto& mesh = resourceManager->getMesh(meshHandle);
  commandBuffer.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
  commandBuffer.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                     vk::ShaderStageFlagBits::eVertex,
                                                     0,
                                                     pushConstants);

  auto& indirectBuffer = resourceManager->getBuffer(frame.getIndexedName("IndirectCommandBuffer"));

  commandBuffer.drawIndexedIndirect(indirectBuffer.getBuffer(),
                                    0,
                                    1,
                                    sizeof(vk::DrawIndexedIndirectCommand));
}

}
