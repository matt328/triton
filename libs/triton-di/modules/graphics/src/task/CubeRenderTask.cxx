#include "CubeRenderTask.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "cm/ObjectData.hpp"
#include "vk/VkResourceManager.hpp"

/*
   - get camera data in from game world
   - set up RenderScheduler to dispatch a compute shader with memory barriers for any involved
   buffers
   - also need to insert a memory barrier before the draw call using the indirect buffer.
*/

namespace tr {

struct InstanceData {
   glm::mat4 model;
};

CubeRenderTask::CubeRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                               std::shared_ptr<GeometryFactory> newGeometryFactory,
                               std::shared_ptr<IndirectPipeline> newPipeline)
    : resourceManager{std::move(newResourceManager)},
      geometryFactory{std::move(newGeometryFactory)},
      pipeline{std::move(newPipeline)} {

   const auto geometryHandle = geometryFactory->createUnitCube();
   meshHandle = resourceManager->asyncUpload(geometryFactory->getGeometryData(geometryHandle));

   const auto commandData =
       vk::DrawIndexedIndirectCommand{.indexCount = 36, // Index Count not Vertex Count
                                      .instanceCount = 1,
                                      .firstIndex = 0,
                                      .vertexOffset = 0,
                                      .firstInstance = 0};

   // The indirectBuffer is never explicitly accessed by shader code.
   indirectBuffer = resourceManager->createBuffer(sizeof(vk::DrawIndexedIndirectCommand),
                                                  vk::BufferUsageFlagBits::eIndirectBuffer |
                                                      vk::BufferUsageFlagBits::eTransferDst,
                                                  "Indirect Command");
   indirectBuffer->mapBuffer();
   indirectBuffer->updateBufferValue(&commandData, sizeof(vk::DrawIndexedIndirectCommand));
   indirectBuffer->unmapBuffer();

   instanceBuffer = resourceManager->createBuffer(sizeof(InstanceData),
                                                  vk::BufferUsageFlagBits::eStorageBuffer |
                                                      vk::BufferUsageFlagBits::eTransferDst |
                                                      vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                                  "InstanceData");

   cameraDataBuffer = resourceManager->createBuffer(
       sizeof(CameraData),
       vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst |
           vk::BufferUsageFlagBits::eShaderDeviceAddress,
       "CameraData");

   const auto position = glm::vec3{0.f, 0.f, 0.f};
   const auto view = glm::lookAt(position, glm::vec3{0.f, 0.f, -5.f}, glm::vec3{0.f, 1.f, 0.f});
   const auto projection =
       glm::perspective(glm::radians(60.f), static_cast<float>(1920 / 1080), 0.1f, 10000.0f);

   const auto cameraData = CameraData{.view = view,
                                      .proj = projection,
                                      .viewProj = view * projection,
                                      .position = glm::vec4{0.f, 0.f, 0.f, 1.f}};

   cameraDataBuffer->mapBuffer();
   cameraDataBuffer->updateBufferValue(&cameraData, sizeof(CameraData));
   cameraDataBuffer->unmapBuffer();

   const auto instanceData =
       InstanceData{.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f))};

   instanceBuffer->mapBuffer();
   instanceBuffer->updateBufferValue(&instanceData, sizeof(InstanceData));
   instanceBuffer->unmapBuffer();

   pushConstants = IndirectPushConstants{.drawID = 0,
                                         .baseAddress = instanceBuffer->getDeviceAddress(),
                                         .cameraDataAddress = cameraDataBuffer->getDeviceAddress()};

   viewport = vk::Viewport{
       .width = 1920,
       .height = 1080,
       .minDepth = 0.f,
       .maxDepth = 1.f,
   };

   snezzor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                        .extent = vk::Extent2D{.width = 1920, .height = 1080}};
}

auto CubeRenderTask::record(vk::raii::CommandBuffer& commandBuffer) -> void {

   // Bind the graphics pipeline
   commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());
   commandBuffer.setViewportWithCount({viewport});
   commandBuffer.setScissorWithCount({snezzor});

   const auto& mesh = resourceManager->getMesh(meshHandle);
   commandBuffer.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
   commandBuffer.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(), 0, vk::IndexType::eUint32);

   commandBuffer.pushConstants<IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                      vk::ShaderStageFlagBits::eVertex,
                                                      0,
                                                      pushConstants);

   commandBuffer.drawIndexedIndirect(indirectBuffer->getBuffer(),
                                     0,
                                     1,
                                     sizeof(vk::DrawIndexedIndirectCommand));
}
}
