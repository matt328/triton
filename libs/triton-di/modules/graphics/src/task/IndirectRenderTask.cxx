#include "IndirectRenderTask.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "task/Frame.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

IndirectRenderTask::IndirectRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                                       std::shared_ptr<IndirectPipeline> newPipeline)
    : resourceManager{std::move(newResourceManager)}, pipeline{std::move(newPipeline)} {
}

/* TODO(matt) Tomorrow get this thing some data to render. Start by debugging RenderDataSystem and
 make sure its sending the right data over.
 Next make sure the MeshBufferManager is doing the right thing and uploading the vertex data to the
 buffer.
 Next make sure it's producing accurate GpuBufferEntries.
 Then make sure the compute shader is producing the correct DrawCommands
 Then rename GpuBufferEntry.instanceID to objectDataId and use that in the vertex shader to hook up
 the model matrix
 Try just having a system rotate the cube entity so the model matrix changes each frame.
 */

auto IndirectRenderTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame)
    -> void {

  auto& objectDataBuffer = resourceManager->getBuffer(frame.getObjectDataBufferHandle());
  auto& cameraDataBuffer = resourceManager->getBuffer(frame.getCameraBufferHandle());

  pushConstants = IndirectPushConstants{.drawID = 0,
                                        .baseAddress = objectDataBuffer.getDeviceAddress(),
                                        .cameraDataAddress = cameraDataBuffer.getDeviceAddress()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  const auto& [vertexBuffer, indexBuffer] = resourceManager->getStaticMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                     vk::ShaderStageFlagBits::eVertex,
                                                     0,
                                                     pushConstants);

  auto& indirectBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());

  commandBuffer.drawIndexedIndirect(indirectBuffer.getBuffer(),
                                    0,
                                    1,
                                    sizeof(vk::DrawIndexedIndirectCommand));
}

}
