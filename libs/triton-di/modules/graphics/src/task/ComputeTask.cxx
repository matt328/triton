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

  auto& instanceDataBuffer = resourceManager->getBuffer(frame.getInstanceDataBufferHandle());
  auto& drawCommandBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());

  const auto& computePipeline = resourceManager->getPipeline(pipelineHandle);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.getPipeline());

  auto computePushConstants = ComputePushConstants{
      .drawCommandBufferAddress = drawCommandBuffer.getDeviceAddress(),
      .instanceDataBufferAddress = instanceDataBuffer.getDeviceAddress(),
      .instanceDataLength = 1,
  };

  commandBuffer.pushConstants<ComputePushConstants>(computePipeline.getPipelineLayout(),
                                                    vk::ShaderStageFlagBits::eCompute,
                                                    0,
                                                    computePushConstants);

  commandBuffer.dispatch(1, 1, 1);
}

}
