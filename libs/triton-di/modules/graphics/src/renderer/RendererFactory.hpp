#pragma once

#include "RendererConfig.hpp"

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::pipe {
   class IShaderCompiler;
}

namespace tr::gfx::rd {

   class IRenderer;

   class RendererFactory {
    public:
      RendererFactory(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                      std::shared_ptr<pipe::IShaderCompiler> newShaderCompiler);
      ~RendererFactory() = default;

      RendererFactory(const RendererFactory&) = default;
      RendererFactory(RendererFactory&&) = delete;
      auto operator=(const RendererFactory&) -> RendererFactory& = default;
      auto operator=(RendererFactory&&) -> RendererFactory& = delete;

      auto createRenderer(RendererConfig config) -> std::shared_ptr<IRenderer>;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<pipe::IShaderCompiler> shaderCompiler;
   };
}
