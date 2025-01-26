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

  // objectData buffer is included in the compute pipeline, but not used yet.
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
