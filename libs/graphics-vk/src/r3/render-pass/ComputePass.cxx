#include "ComputePass.hpp"

namespace tr {

ComputePass::ComputePass(ComputePassConfig&& newConfig,
                         std::shared_ptr<ImageManager> newImageManager)
    : imageManager{std::move(newImageManager)}, config{std::move(newConfig)} {
}

auto ComputePass::dispatch(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) const -> void {

  cmdBuffer.begin(vk::CommandBufferBeginInfo{});
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, config.pipeline);

  cmdBuffer.dispatch(1024, 1, 1);
  cmdBuffer.end();
}

}
