#include "ComputeTask.hpp"
#include "Frame.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "pipeline/IPipeline.hpp"

namespace tr {

ComputeTask::ComputeTask(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)},
      pipelineHandle{resourceManager->createComputePipeline("Compute")} {
}

auto ComputeTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame& frame) -> void {

  auto& gpuBufferEntryBuffer = resourceManager->getBuffer(frame.getGpuBufferEntryBufferHandle());
  auto& objectDataBuffer = resourceManager->getBuffer(frame.getObjectDataBufferHandle());
  auto& drawCommandBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());

  const auto& computePipeline = resourceManager->getPipeline(pipelineHandle);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.getPipeline());

  // objectData buffer is included in the compute pipeline, but not used yet.
  auto computePushConstants = ComputePushConstants{
      .drawCommandBufferAddress = drawCommandBuffer.getDeviceAddress(),
      .gpuBufferEntryBufferAddress = gpuBufferEntryBuffer.getDeviceAddress(),
      .objectDataBufferAddress = objectDataBuffer.getDeviceAddress(),
      .instanceDataLength = 0,
  };

  commandBuffer.pushConstants<ComputePushConstants>(computePipeline.getPipelineLayout(),
                                                    vk::ShaderStageFlagBits::eCompute,
                                                    0,
                                                    computePushConstants);

  commandBuffer.dispatch(1, 1, 1);
}

}
