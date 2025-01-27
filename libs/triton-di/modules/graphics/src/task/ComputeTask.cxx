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

    - Frames are getting a bit busy with getters and setters for Buffers. Figure out how to clean
    that up a bit.
      - Frame class could define an enum for BufferTypes, and just have setBufferHandle(type,
      handle) and getBufferHandle(type)

    - Adjust the asset lib to differentiate static models from animated ones so that static models
    use the smaller vertex format, and animated models use the format including joints and weights.
    - Might need a different file format for tsm static and tam animated when loading models, the
    engine will need to know which type it is loading.
    - Could also keep the file the same, but have a flag inside to differentiate between animated
    and static. The in memory model classes would have to track this also so that the editor doesn't
    allow attaching a skeleton and animations to a model with no skinning data.

    - Create a utility function to create and record a memory barrier for a given buffer.

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

auto ComputeTask::record(vk::raii::CommandBuffer& commandBuffer,
                         const ComputePushConstants& pushConstants) -> void {
  const auto& computePipeline = resourceManager->getPipeline(pipelineHandle);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.getPipeline());

  commandBuffer.pushConstants<ComputePushConstants>(computePipeline.getPipelineLayout(),
                                                    vk::ShaderStageFlagBits::eCompute,
                                                    0,
                                                    pushConstants);

  commandBuffer.dispatch(1024, 1, 1);
}

}
