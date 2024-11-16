#include "RendererFactory.hpp"
#include "renderer/RendererFactory.hpp"
#include "renderer/RendererConfig.hpp"
#include "DefaultRenderer.hpp"

namespace tr::gfx::rd {

   RendererFactory::RendererFactory(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                                    std::shared_ptr<pipe::IShaderCompiler> newShaderCompiler)
       : graphicsDevice{std::move(newGraphicsDevice)},
         shaderCompiler{std::move(newShaderCompiler)} {
   }

   auto RendererFactory::createRenderer(RendererConfig config) -> std::shared_ptr<IRenderer> {
      if (config.rendererType == RendererType::StaticModel) {
         return std::make_shared<DefaultRenderer>(config, graphicsDevice, shaderCompiler);
      }
      return nullptr;
   }
}