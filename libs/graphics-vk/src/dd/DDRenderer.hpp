#pragma once

#include "gfx/IRenderContext.hpp"

namespace tr {

using RenderConfigHandle = uint64_t;

struct Renderable {
  RenderConfigHandle renderConfigHandle;
  GeometryHandle geometryHandle;
  MaterialHandle materialHandle;
  ObjectID objectId;
};

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
  std::unordered_map<RenderConfigHandle, std::vector<Renderable>> categorizedRenderables;
};

}
