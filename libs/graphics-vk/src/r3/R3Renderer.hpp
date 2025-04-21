#pragma once

#include "gfx/IRenderContext.hpp"

namespace tr {

class R3Renderer : public tr::IRenderContext {
public:
  R3Renderer() = default;
  ~R3Renderer() override = default;

  R3Renderer(const R3Renderer&) = default;
  R3Renderer(R3Renderer&&) = delete;
  auto operator=(const R3Renderer&) -> R3Renderer& = default;
  auto operator=(R3Renderer&&) -> R3Renderer& = delete;

  auto registerRenderable(const RenderableData& data) -> RenderableResources override;

  void update() override;
  void renderNextFrame() override;
  void waitIdle() override;

  void setRenderData(const RenderData& renderData) override;
};
}
