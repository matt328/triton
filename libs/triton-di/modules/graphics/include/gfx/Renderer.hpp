#pragma once

#include "IRenderer.hpp"

namespace tr::gfx {

   class IGraphicsDevice;

   namespace sb {
      class LayoutFactory;
      class ShaderBindingFactory;
   }

   class Renderer : public IRenderer {
    public:
      explicit Renderer(std::shared_ptr<IGraphicsDevice> graphicsDevice);
      ~Renderer() override = default;

      void render() override;
      void waitIdle() override;

    private:
      std::shared_ptr<sb::LayoutFactory> layoutFactory;
      std::shared_ptr<sb::ShaderBindingFactory> shaderBindingFactory;
   };
}
