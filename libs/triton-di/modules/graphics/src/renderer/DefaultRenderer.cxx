#include "DefaultRenderer.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "pipeline/StaticModelPipeline.hpp"

namespace tr::gfx::rd {
   DefaultRenderer::DefaultRenderer(const RendererConfig& config,
                                    const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
                                    const std::shared_ptr<pipe::IShaderCompiler>& shaderCompiler) {
      Log.trace("Constructing DefaultRenderer");
      pipeline =
          std::make_shared<pipe::StaticModelPipeline>(graphicsDevice, shaderCompiler, config);
   }
}