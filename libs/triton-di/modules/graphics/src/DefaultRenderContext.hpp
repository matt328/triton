#pragma once

#include "tr/IEventBus.hpp"
#include "tr/IGuiSystem.hpp"
#include "gfx/IRenderContext.hpp"
#include "tr/IGameplaySystem.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "renderer/RendererFactory.hpp"
#include "sb/ILayoutFactory.hpp"
#include "sb/IShaderBindingFactory.hpp"

#include "cm/RenderData.hpp"

namespace tr {

class IGraphicsDevice;
class DepthResources;
class FrameManager;

namespace rd {
class RendererFactory;
}

class DefaultRenderContext final : public IRenderContext {
public:
  explicit DefaultRenderContext(std::shared_ptr<IGraphicsDevice> graphicsDevice,
                                std::shared_ptr<ILayoutFactory> newLayoutFactory,
                                std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                                std::shared_ptr<IShaderCompiler> newShaderCompiler,
                                std::shared_ptr<rd::RendererFactory> newRendererFactory,
                                std::shared_ptr<IGameplaySystem> newGameplaySystem,
                                std::shared_ptr<tr::IGuiSystem> newGuiSystem,
                                std::shared_ptr<IEventBus> newEventBus);
  ~DefaultRenderContext() override;

  DefaultRenderContext(const DefaultRenderContext&) = delete;
  DefaultRenderContext(DefaultRenderContext&&) = delete;
  auto operator=(const DefaultRenderContext&) -> DefaultRenderContext& = delete;
  auto operator=(DefaultRenderContext&&) -> DefaultRenderContext& = delete;

  void render() override;
  void waitIdle() override;
  void setRenderData(const RenderData& renderData) override;

private:
  std::shared_ptr<IGraphicsDevice> graphicsDevice;
  std::shared_ptr<ILayoutFactory> layoutFactory;
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<IShaderCompiler> shaderCompiler;
  std::shared_ptr<rd::RendererFactory> rendererFactory;
  std::shared_ptr<IGameplaySystem> gameplaySystem;
  std::shared_ptr<tr::IGuiSystem> guiSystem;
  std::shared_ptr<IEventBus> eventBus;

  std::shared_ptr<FrameManager> frameManager;

  std::shared_ptr<rd::IRenderer> defaultRenderer;

  std::shared_ptr<DepthResources> depthResources;

  mutable TracyLockable(std::mutex, renderDataMutex);
  RenderData renderData;

  [[nodiscard]] auto getViewportAndScissor() -> std::tuple<vk::Viewport, vk::Rect2D>;
};
}
