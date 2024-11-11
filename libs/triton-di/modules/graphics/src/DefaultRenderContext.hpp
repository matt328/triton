#pragma once

#include "DepthResources.hpp"
#include "gfx/IRenderContext.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "sb/ILayoutFactory.hpp"
#include "sb/IShaderBindingFactory.hpp"
#include "renderer/RendererFactory.hpp"

namespace tr::gfx {

   class IGraphicsDevice;
   class DepthResources;
   class FrameManager;

   namespace sb {
      class LayoutFactory;
      class ShaderBindingFactory;
   }

   class DefaultRenderContext : public IRenderContext {
    public:
      explicit DefaultRenderContext(
          std::shared_ptr<IGraphicsDevice> graphicsDevice,
          std::shared_ptr<sb::ILayoutFactory> newLayoutFactory,
          std::shared_ptr<sb::IShaderBindingFactory> newShaderBindingFactory,
          std::shared_ptr<pipe::IShaderCompiler> newShaderCompiler,
          std::shared_ptr<rd::RendererFactory> newRendererFactory);
      ~DefaultRenderContext() override = default;

      DefaultRenderContext(const DefaultRenderContext&) = default;
      DefaultRenderContext(DefaultRenderContext&&) = delete;
      auto operator=(const DefaultRenderContext&) -> DefaultRenderContext& = default;
      auto operator=(DefaultRenderContext&&) -> DefaultRenderContext& = delete;

      void render() override;
      void waitIdle() override;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<sb::ILayoutFactory> layoutFactory;
      std::shared_ptr<sb::IShaderBindingFactory> shaderBindingFactory;
      std::shared_ptr<pipe::IShaderCompiler> shaderCompiler;
      std::shared_ptr<rd::RendererFactory> rendererFactory;

      std::shared_ptr<rd::IRenderer> defaultRenderer;

      std::shared_ptr<DepthResources> depthResources;

      std::shared_ptr<FrameManager> frameManager;

      [[nodiscard]] auto getViewportAndScissor() -> std::tuple<vk::Viewport, vk::Rect2D>;
   };
}
