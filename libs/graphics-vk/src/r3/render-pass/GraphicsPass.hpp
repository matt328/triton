#pragma once

#include "r3/render-pass/GraphicsPassConfig.hpp"
#include "r3/render-pass/BindFunctions.hpp"

namespace tr {

class ImageManager;
class Frame;

class GraphicsPass {
public:
  GraphicsPass(GraphicsPassConfig&& config, std::shared_ptr<ImageManager> newImageManager);
  ~GraphicsPass() = default;

  GraphicsPass(const GraphicsPass&) = default;
  GraphicsPass(GraphicsPass&&) = delete;
  auto operator=(const GraphicsPass&) -> GraphicsPass& = default;
  auto operator=(GraphicsPass&&) -> GraphicsPass& = delete;

  auto bind(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer, PushConstantsBindFn& bindFn)
      -> void;

  auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void;

private:
  std::shared_ptr<ImageManager> imageManager;
};

}
