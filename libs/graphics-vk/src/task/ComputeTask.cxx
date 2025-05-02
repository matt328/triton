#include "ComputeTask.hpp"
#include "Frame.hpp"
#include "vk/ComputePushConstants.hpp"
#include "pipeline/IPipeline.hpp"

namespace tr {

ComputeTask::ComputeTask(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)},
      pipelineHandle{resourceManager->createComputePipeline("Compute")} {
}

auto ComputeTask::record([[maybe_unused]] vk::raii::CommandBuffer& commandBuffer,
                         [[maybe_unused]] const Frame* frame) -> void {
}

auto ComputeTask::record(vk::raii::CommandBuffer& commandBuffer,
                         const ComputePushConstants& pushConstants) -> void {
  const auto& computePipeline = resourceManager->getPipeline(pipelineHandle);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.getPipeline());

  // commandBuffer.pushConstants(computePipeline.getPipelineLayout(),
  //                             vk::ShaderStageFlagBits::eCompute,
  //                             0,
  //                             {pushConstants});

  commandBuffer.dispatch(1024, 1, 1);
}

}
