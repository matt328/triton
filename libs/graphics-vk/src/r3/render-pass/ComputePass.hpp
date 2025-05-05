#pragma once

#include "r3/render-pass/BindFunctions.hpp"
#include "r3/render-pass/ComputePassConfig.hpp"

namespace tr {

class Frame;
class ImageManager;

class ComputePass {
public:
  ComputePass(ComputePassConfig&& newConfig, std::shared_ptr<ImageManager> newImageManager);
  ~ComputePass() = default;

  ComputePass(const ComputePass&) = delete;
  ComputePass(ComputePass&&) = delete;
  auto operator=(const ComputePass&) -> ComputePass& = delete;
  auto operator=(ComputePass&&) -> ComputePass& = delete;

  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void;

  [[nodiscard]] auto getId() const {
    return config.id;
  }

private:
  std::shared_ptr<ImageManager> imageManager;
  ComputePassConfig config;
};

}
