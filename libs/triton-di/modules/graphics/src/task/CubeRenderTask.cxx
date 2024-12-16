#include "CubeRenderTask.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "cm/ObjectData.hpp"
#include "vk/VkResourceManager.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx::task {

   struct IndirectCommand {
      uint32_t vertexCount;
      uint32_t instanceCount;
      uint32_t firstVertex;
      uint32_t firstInstance;
      std::array<uint8_t, 4> padding; // Vulkan wants this buffer's stride to be 20 bytes.
   };

   struct InstanceData {
      glm::mat4 model;
   };

   CubeRenderTask::CubeRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                                  std::shared_ptr<geo::GeometryFactory> newGeometryFactory,
                                  std::shared_ptr<pipe::IndirectPipeline> newPipeline)
       : resourceManager{std::move(newResourceManager)},
         geometryFactory{std::move(newGeometryFactory)},
         pipeline{std::move(newPipeline)} {

      const auto geometryHandle = geometryFactory->createUnitCube();
      meshHandle = resourceManager->asyncUpload(geometryFactory->getGeometryData(geometryHandle));

      const auto commandData = IndirectCommand{.vertexCount = 36, // Index Count not Vertex Count
                                               .instanceCount = 1,
                                               .firstVertex = 0,
                                               .firstInstance = 0};

      // The indirectBuffer is never actually referenced by any shader code.
      indirectBuffer = resourceManager->createBuffer(sizeof(IndirectCommand),
                                                     vk::BufferUsageFlagBits::eIndirectBuffer |
                                                         vk::BufferUsageFlagBits::eTransferDst,
                                                     "Indirect Command");
      indirectBuffer->mapBuffer();
      indirectBuffer->updateBufferValue(&commandData, sizeof(IndirectCommand));
      indirectBuffer->unmapBuffer();

      instanceBuffer = resourceManager->createBuffer(
          sizeof(InstanceData),
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst |
              vk::BufferUsageFlagBits::eShaderDeviceAddress,
          "InstanceData");

      cameraDataBuffer = resourceManager->createBuffer(
          sizeof(cm::gpu::CameraData),
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst |
              vk::BufferUsageFlagBits::eShaderDeviceAddress,
          "CameraData");

      const auto position = glm::vec3{0.f, 0.f, 0.f};
      const auto view = glm::lookAt(position, glm::vec3{0.f, 0.f, -5.f}, glm::vec3{0.f, 1.f, 0.f});
      const auto projection =
          glm::perspective(glm::radians(60.f), static_cast<float>(1920 / 1080), 0.1f, 10000.0f);

      const auto cameraData = cm::gpu::CameraData{.view = view,
                                                  .proj = projection,
                                                  .viewProj = view * projection,
                                                  .position = glm::vec4{0.f, 0.f, 0.f, 1.f}};

      cameraDataBuffer->mapBuffer();
      cameraDataBuffer->updateBufferValue(&cameraData, sizeof(cm::gpu::CameraData));
      cameraDataBuffer->unmapBuffer();

      const auto instanceData =
          InstanceData{.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f))};

      instanceBuffer->mapBuffer();
      instanceBuffer->updateBufferValue(&instanceData, sizeof(InstanceData));
      instanceBuffer->unmapBuffer();

      pushConstants =
          cm::gpu::IndirectPushConstants{.drawID = 0,
                                         .baseAddress = instanceBuffer->getDeviceAddress(),
                                         .cameraDataAddress = cameraDataBuffer->getDeviceAddress()};

      viewport = vk::Viewport{
          .width = 1920,
          .height = 1080,
          .minDepth = 0.f,
          .maxDepth = 1.f,
      };

      snezzor = vk::Rect2D{.offset = vk::Offset2D{0, 0},
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

      commandBuffer.pushConstants<cm::gpu::IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                                  vk::ShaderStageFlagBits::eVertex,
                                                                  0,
                                                                  pushConstants);

      commandBuffer.drawIndexedIndirect(indirectBuffer->getBuffer(), 0, 1, sizeof(IndirectCommand));
   }
}
