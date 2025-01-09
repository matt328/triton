#include "IndirectRenderTask.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "task/Frame.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

IndirectRenderTask::IndirectRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                                       std::shared_ptr<IndirectPipeline> newPipeline)
    : resourceManager{std::move(newResourceManager)}, pipeline{std::move(newPipeline)} {
}

auto IndirectRenderTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame)
    -> void {

  auto& objectDataBuffer = resourceManager->getBuffer(frame.getObjectDataBufferHandle());
  auto& cameraDataBuffer = resourceManager->getBuffer(frame.getCameraBufferHandle());

  pushConstants = IndirectPushConstants{.drawID = 0,
                                        .objectDataAddress = objectDataBuffer.getDeviceAddress(),
                                        .cameraDataAddress = cameraDataBuffer.getDeviceAddress(),
                                        .objectDataLength = 2};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  const auto& [vertexBuffer, indexBuffer] = resourceManager->getStaticMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                     vk::ShaderStageFlagBits::eVertex,
                                                     0,
                                                     pushConstants);

  [[maybe_unused]] vk::DrawIndexedIndirectCommand f{};

  auto& indirectBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());
  auto& countBuffer = resourceManager->getBuffer(frame.getCountBufferHandle());

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         2,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
