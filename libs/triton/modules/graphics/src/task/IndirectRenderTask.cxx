#include "IndirectRenderTask.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "task/Frame.hpp"
#include "vk/BufferManager.hpp"
#include "vk/VkResourceManager.hpp"
#include "vk/sb/IShaderBinding.hpp"

namespace tr {

IndirectRenderTask::IndirectRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                                       std::shared_ptr<IndirectPipeline> newPipeline,
                                       std::shared_ptr<BufferManager> newBufferManager,
                                       RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      pipeline{std::move(newPipeline)},
      bufferManager{std::move(newBufferManager)},
      config{newConfig} {
}

auto IndirectRenderTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame)
    -> void {

  const auto objectDataAddress =
      bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicObjectDataBuffer))
          .getDeviceAddress();
  const auto cameraDataAddress =
      bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::CameraBuffer))
          .getDeviceAddress();
  const auto& objectDataIndexAddress =
      bufferManager
          ->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicObjectDataIndexBuffer))
          .getDeviceAddress();

  const auto handle = frame.getBufferHandle(BufferHandleType::AnimationDataBuffer);
  const auto& animationDataBuffer = bufferManager->getBuffer(handle);
  const auto& animationDataBufferAddress = animationDataBuffer.getDeviceAddress();

  pushConstants = IndirectPushConstants{.objectDataAddress = objectDataAddress,
                                        .cameraDataAddress = cameraDataAddress,
                                        .objectDataIndexAddress = objectDataIndexAddress,
                                        .animationDataAddress = animationDataBufferAddress,
                                        .drawID = 0,
                                        .objectCount = frame.getSkinnedObjectCount()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  auto& textureShaderBinding = resourceManager->getTextureShaderBinding();

  textureShaderBinding.bindToPipeline(commandBuffer,
                                      vk::PipelineBindPoint::eGraphics,
                                      0,
                                      pipeline->getPipelineLayout());

  const auto& [vertexBuffer, indexBuffer] = resourceManager->getSkinnedMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                     vk::ShaderStageFlagBits::eVertex,
                                                     0,
                                                     pushConstants);

  auto& indirectBuffer =
      bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicDrawCommand));
  auto& countBuffer =
      bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicCountBuffer));

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         config.maxStaticObjects,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
