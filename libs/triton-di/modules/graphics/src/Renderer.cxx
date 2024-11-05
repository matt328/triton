#include "Renderer.hpp"
#include "sb/LayoutFactory.hpp"

namespace tr::gfx {

   Renderer::Renderer(std::shared_ptr<IGraphicsDevice> graphicsDevice) {
      const auto config = sb::Config{
          .useDescriptorBuffers = true,
          .maxTextures = 16,
      };
      layoutFactory = std::make_shared<sb::LayoutFactory>(graphicsDevice, config);
   }

   void Renderer::render() {
   }

   void Renderer::waitIdle() {
   }
}