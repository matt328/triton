#pragma once

#include "gfx/IRenderContext.hpp"
#include "dd/Renderable.hpp"

namespace tr {

class DDRenderer : public IRenderContext {
public:
  DDRenderer() = default;
  ~DDRenderer() override = default;

  DDRenderer(const DDRenderer&) = default;
  DDRenderer(DDRenderer&&) = delete;
  auto operator=(const DDRenderer&) -> DDRenderer& = default;
  auto operator=(DDRenderer&&) -> DDRenderer& = delete;

  auto update() -> void override;
  auto renderNextFrame() -> void override;
  auto waitIdle() -> void override;
  auto setRenderData(const RenderData& newRenderData) -> void override;

private:
  RenderData renderData;
  std::vector<Renderable> renderables;
  std::unordered_map<RenderConfigHandle, std::vector<Renderable>> categorizedRenderables;
};

}
