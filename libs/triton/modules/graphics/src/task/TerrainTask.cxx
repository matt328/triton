#include "TerrainTask.hpp"
#include "task/Frame.hpp"
#include "vk/BufferManager.hpp"
#include "vk/sb/IShaderBinding.hpp"

namespace tr {

TerrainTask::TerrainTask(std::shared_ptr<VkResourceManager> newResourceManager,
                         std::shared_ptr<StaticPipeline> newPipeline,
                         std::shared_ptr<BufferManager> newBufferManager,
                         RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      pipeline{std::move(newPipeline)},
      bufferManager{std::move(newBufferManager)},
      config{newConfig} {
}

auto TerrainTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame* frame) -> void {

  const auto objectDataAddress =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::TerrainChunkData))
          .getDeviceAddress();
  const auto cameraDataAddress =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::CameraBuffer))
          .getDeviceAddress();
  const auto& objectDataIndexAddress =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::TerrainChunkDataIndex))
          .getDeviceAddress();

  pushConstants = StaticPushConstants{.drawID = 0,
                                      .objectDataAddress = objectDataAddress,
                                      .cameraDataAddress = cameraDataAddress,
                                      .objectDataIndexAddress = objectDataIndexAddress,
                                      .objectCount = frame->getTerrainChunkCount()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  auto& textureShaderBinding = resourceManager->getTextureShaderBinding();

  textureShaderBinding.bindToPipeline(commandBuffer,
                                      vk::PipelineBindPoint::eGraphics,
                                      0,
                                      pipeline->getPipelineLayout());

  const auto& [vertexBuffer, indexBuffer] = resourceManager->getTerrainMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<StaticPushConstants>(pipeline->getPipelineLayout(),
                                                   vk::ShaderStageFlagBits::eVertex,
                                                   0,
                                                   pushConstants);

  auto& indirectBuffer =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::TerrainChunkDrawCommand));
  auto& countBuffer =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::TerrainChunkCount));

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         config.maxTerrainChunks,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
