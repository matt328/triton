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
                         [[maybe_unused]] const Frame& frame) -> void {

  /*
    - Still need to figure out a clean way to pass the actual object counts to Tasks, and to the
    RenderScheduler in general.
      - renderData contains this info in the sizes of its lists. Probably have to store this off in
      the frame to reference in the RenderScheduler and/or the RenderTasks.

    - Adjust the asset lib to differentiate static models from animated ones so that static models
    use the smaller vertex format, and animated models use the format including joints and weights.
    - Could use a different file format for tsm static and tam animated when loading models, the
    engine will need to know which type it is loading.
    - Could also keep the file the same, but have a flag inside to differentiate between animated
    and static. The in memory model classes would have to track this also so that the editor doesn't
    allow attaching a skeleton and animations to a model with no skinning data.

  */
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
