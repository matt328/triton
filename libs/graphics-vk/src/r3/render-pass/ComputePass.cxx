#include "ComputePass.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"

namespace tr {

ComputePass::ComputePass(ComputePassConfig&& newConfig,
                         std::shared_ptr<ImageManager> newImageManager,
                         std::shared_ptr<ContextFactory> newContextFactory)
    : imageManager{std::move(newImageManager)},
      config{std::move(newConfig)},
      contextFactory{std::move(newContextFactory)} {
}

auto ComputePass::registerDispatchContext(Handle<IDispatchContext> handle) -> void {
  dispatchableContexts.push_back(handle);
}

auto ComputePass::dispatch(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) const -> void {

  cmdBuffer.begin(vk::CommandBufferBeginInfo{});
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, config.pipeline);

  for (const auto& handle : dispatchableContexts) {
    const auto& dispatchContext = contextFactory->getDispatchContext(handle);
    dispatchContext->bind(frame, cmdBuffer, config.pipelineLayout);
    dispatchContext->dispatch(frame, cmdBuffer);
  }

  cmdBuffer.end();
}

}
