#pragma once

#include "dd/DispatchContext.hpp"
#include "r3/render-pass/BindFunctions.hpp"
#include "r3/render-pass/ComputePassConfig.hpp"

namespace tr {

class DispatchContext;
class Frame;
class ImageManager;

class ComputePass {
public:
  explicit ComputePass(ComputePassConfig&& newConfig, std::shared_ptr<ImageManager> newImageManager)
      : imageManager{std::move(newImageManager)}, config{std::move(newConfig)} {
  }
  ~ComputePass() = default;

  ComputePass(const ComputePass&) = delete;
  ComputePass(ComputePass&&) = delete;
  auto operator=(const ComputePass&) -> ComputePass& = delete;
  auto operator=(ComputePass&&) -> ComputePass& = delete;

  auto bind(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer, PushConstantsBindFn& bindFn)
      -> void {
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, config.pipeline);
    bindFn(frame, cmdBuffer);
  }

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
    dispatchContext->dispatch();
  }

private:
  std::shared_ptr<ImageManager> imageManager;
  ComputePassConfig config;
  std::unique_ptr<DispatchContext> dispatchContext;
};

}
