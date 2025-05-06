#pragma once

#include "bk/Handle.hpp"
#include "r3/render-pass/BindFunctions.hpp"
#include "r3/render-pass/ComputePassConfig.hpp"

namespace tr {

class Frame;
class ImageManager;
class DispatchContext;
class ContextFactory;

class ComputePass {
public:
  ComputePass(ComputePassConfig&& newConfig,
              std::shared_ptr<ImageManager> newImageManager,
              std::shared_ptr<ContextFactory> newContextFactory);
  ~ComputePass() = default;

  ComputePass(const ComputePass&) = delete;
  ComputePass(ComputePass&&) = delete;
  auto operator=(const ComputePass&) -> ComputePass& = delete;
  auto operator=(ComputePass&&) -> ComputePass& = delete;

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) const -> void;

  auto registerDispatchContext(Handle<DispatchContext> handle) -> void;

  [[nodiscard]] auto getId() const {
    return config.id;
  }

private:
  std::shared_ptr<ImageManager> imageManager;
  ComputePassConfig config;
  std::shared_ptr<ContextFactory> contextFactory;

  std::vector<Handle<DispatchContext>> dispatchableContexts;
};

}
