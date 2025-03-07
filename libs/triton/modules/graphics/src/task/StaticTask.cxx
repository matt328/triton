#include "StaticTask.hpp"
#include "task/Frame.hpp"
#include "vk/BufferManager.hpp"
#include "vk/sb/IShaderBinding.hpp"

namespace tr {

StaticTask::StaticTask(std::shared_ptr<VkResourceManager> newResourceManager,
                       std::shared_ptr<StaticPipeline> newPipeline,
                       std::shared_ptr<BufferManager> newBufferManager,
                       RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      pipeline{std::move(newPipeline)},
      bufferManager{std::move(newBufferManager)},
      config{newConfig} {
}

auto StaticTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame* frame) -> void {

  const auto objectDataAddress =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::StaticObjectDataBuffer))
          .getDeviceAddress();
  const auto cameraDataAddress =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::CameraBuffer))
          .getDeviceAddress();
  const auto& objectDataIndexAddress =
      bufferManager
          ->getBuffer(frame->getBufferHandle(BufferHandleType::StaticObjectDataIndexBuffer))
          .getDeviceAddress();

  pushConstants = StaticPushConstants{.drawID = 0,
                                      .objectDataAddress = objectDataAddress,
                                      .cameraDataAddress = cameraDataAddress,
                                      .objectDataIndexAddress = objectDataIndexAddress,
                                      .objectCount = frame->getStaticObjectCount()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  auto& textureShaderBinding = resourceManager->getTextureShaderBinding();

  textureShaderBinding.bindToPipeline(commandBuffer,
                                      vk::PipelineBindPoint::eGraphics,
                                      0,
                                      pipeline->getPipelineLayout());

  const auto& [vertexBuffer, indexBuffer] = resourceManager->getStaticMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<StaticPushConstants>(pipeline->getPipelineLayout(),
                                                   vk::ShaderStageFlagBits::eVertex,
                                                   0,
                                                   pushConstants);

  auto& indirectBuffer =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::StaticDrawCommand));
  auto& countBuffer =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::StaticCountBuffer));

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         config.maxStaticObjects,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
