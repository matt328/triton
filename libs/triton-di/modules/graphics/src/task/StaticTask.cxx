#include "StaticTask.hpp"
#include "task/Frame.hpp"
#include "vk/sb/IShaderBinding.hpp"

namespace tr {

StaticTask::StaticTask(std::shared_ptr<VkResourceManager> newResourceManager,
                       std::shared_ptr<StaticPipeline> newPipeline,
                       RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      pipeline{std::move(newPipeline)},
      config{newConfig} {
}

auto StaticTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void {

  const auto objectDataAddress =
      resourceManager->getBuffer(frame.getGpuObjectDataBufferHandle()).getDeviceAddress();
  const auto cameraDataAddress =
      resourceManager->getBuffer(frame.getCameraBufferHandle()).getDeviceAddress();
  const auto& objectDataIndexAddress =
      resourceManager->getBuffer(frame.getObjectDataIndexBufferHandle()).getDeviceAddress();

  pushConstants = StaticPushConstants{.drawID = 0,
                                      .objectDataAddress = objectDataAddress,
                                      .cameraDataAddress = cameraDataAddress,
                                      .objectDataIndexAddress = objectDataIndexAddress};

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

  auto& indirectBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());
  auto& countBuffer = resourceManager->getBuffer(frame.getCountBufferHandle());

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         config.maxStaticObjects,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
