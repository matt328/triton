#pragma once

#include "gfx/IRenderContext.hpp"
#include "tr/IGameplaySystem.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "renderer/RendererFactory.hpp"
#include "sb/ILayoutFactory.hpp"
#include "sb/IShaderBindingFactory.hpp"

#include "cm/RenderData.hpp"

namespace tr::gfx {

   class IGraphicsDevice;
   class DepthResources;
   class FrameManager;

   namespace rd {
      class RendererFactory;
   }

   class DefaultRenderContext : public IRenderContext {
    public:
      explicit DefaultRenderContext(
          std::shared_ptr<IGraphicsDevice> graphicsDevice,
          std::shared_ptr<sb::ILayoutFactory> newLayoutFactory,
          std::shared_ptr<sb::IShaderBindingFactory> newShaderBindingFactory,
          std::shared_ptr<pipe::IShaderCompiler> newShaderCompiler,
          std::shared_ptr<rd::RendererFactory> newRendererFactory,
          std::shared_ptr<gp::IGameplaySystem> newGameplaySystem);
      ~DefaultRenderContext() override;

      DefaultRenderContext(const DefaultRenderContext&) = delete;
      DefaultRenderContext(DefaultRenderContext&&) = delete;
      auto operator=(const DefaultRenderContext&) -> DefaultRenderContext& = delete;
      auto operator=(DefaultRenderContext&&) -> DefaultRenderContext& = delete;

      void render() override;
      void waitIdle() override;
      void setRenderData(const cm::gpu::RenderData& renderData) override;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<sb::ILayoutFactory> layoutFactory;
      std::shared_ptr<sb::IShaderBindingFactory> shaderBindingFactory;
      std::shared_ptr<pipe::IShaderCompiler> shaderCompiler;
      std::shared_ptr<rd::RendererFactory> rendererFactory;
      std::shared_ptr<gp::IGameplaySystem> gameplaySystem;

      std::shared_ptr<FrameManager> frameManager;

      std::shared_ptr<rd::IRenderer> defaultRenderer;

      std::shared_ptr<DepthResources> depthResources;

      mutable TracyLockable(std::mutex, renderDataMutex);
      cm::gpu::RenderData renderData;

      [[nodiscard]] auto getViewportAndScissor() -> std::tuple<vk::Viewport, vk::Rect2D>;
      void resizeSwapchain();
   };
}
