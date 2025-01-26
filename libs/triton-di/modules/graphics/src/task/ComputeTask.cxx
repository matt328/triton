#include "ComputeTask.hpp"
#include "Frame.hpp"
#include "vk/ComputePushConstants.hpp"
#include "pipeline/IPipeline.hpp"

namespace tr {

ComputeTask::ComputeTask(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)},
      pipelineHandle{resourceManager->createComputePipeline("Compute")} {
}

auto ComputeTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void {

  const auto& gpuBufferEntryBuffer =
      resourceManager->getBuffer(frame.getGpuBufferEntryBufferHandle());
  const auto& objectDataBuffer = resourceManager->getBuffer(frame.getGpuObjectDataBufferHandle());
  const auto& drawCommandBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());
  const auto& countBuffer = resourceManager->getBuffer(frame.getCountBufferHandle());
  const auto& objectDataIndexBuffer =
      resourceManager->getBuffer(frame.getObjectDataIndexBufferHandle());

  const auto& computePipeline = resourceManager->getPipeline(pipelineHandle);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.getPipeline());

  /*
    - Still need to figure out a clean way to pass the actual object counts to Tasks, and to the
    RenderScheduler in general.

    - If we are doing separate draw calls, decide whether the complexity of a shared ObjectData
    buffer and Vertex/Index buffers is worth it, or should different pipelines have their own
    ObjectData and Vertex/Index buffers.
    - It would be more effective to have static models be trimmed down in their ObjectData as well
    as their vertex sizes, they don't need skinning data.

    - Frames are getting a bit busy with getters and setters for Buffers. Figure out how to clean
    that up a bit.

    - Create another pipeline, render task, and ObjectDataIndex buffer for animated models.

    - Adjust the compute shader so it fills out the various ObjectDataIndex buffers.

    - Adjust the vertex and fragment shaders of RenderTasks so they use the ObjectDataIndexBuffer to
    look up the Object's data.

  */

  auto computePushConstants =
      ComputePushConstants{.drawCommandBufferAddress = drawCommandBuffer.getDeviceAddress(),
                           .gpuBufferEntryBufferAddress = gpuBufferEntryBuffer.getDeviceAddress(),
                           .objectDataBufferAddress = objectDataBuffer.getDeviceAddress(),
                           .countBufferAddress = countBuffer.getDeviceAddress(),
                           .objectDataIndexBufferAddress = objectDataIndexBuffer.getDeviceAddress(),
                           .objectCount = 2};

  commandBuffer.pushConstants<ComputePushConstants>(computePipeline.getPipelineLayout(),
                                                    vk::ShaderStageFlagBits::eCompute,
                                                    0,
                                                    computePushConstants);

  commandBuffer.dispatch(1024, 1, 1);
}

}
