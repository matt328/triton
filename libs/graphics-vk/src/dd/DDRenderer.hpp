#pragma once

#include "dd/RenderPass.hpp"
#include "gfx/IRenderContext.hpp"
#include "dd/Renderable.hpp"

namespace tr {

class RenderConfigRegistry;
class DrawContextFactory;
class IFrameManager;

class DDRenderer : public IRenderContext {
public:
  explicit DDRenderer(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                      std::shared_ptr<DrawContextFactory> newDrawContextFactory,
                      std::shared_ptr<IFrameManager> newFrameManager);
  ~DDRenderer() override = default;

  DDRenderer(const DDRenderer&) = default;
  DDRenderer(DDRenderer&&) = delete;
  auto operator=(const DDRenderer&) -> DDRenderer& = default;
  auto operator=(DDRenderer&&) -> DDRenderer& = delete;

  auto update() -> void override;
  auto renderNextFrame() -> void override;
  auto waitIdle() -> void override;
  auto setRenderData(const RenderData& newRenderData) -> void override;

  auto registerRenderable(const RenderableData& data) -> RenderableResources override;

private:
  std::shared_ptr<RenderConfigRegistry> renderConfigRegistry;
  std::shared_ptr<DrawContextFactory> drawContextFactory;
  std::shared_ptr<IFrameManager> frameManager;

  std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> renderPasses;

  RenderData renderData;
  std::vector<Renderable> renderables;
  std::unordered_map<RenderConfigHandle, std::vector<Renderable>> categorizedRenderables;
};

}
