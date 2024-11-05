#include "Renderer.hpp"
#include "sb/LayoutFactory.hpp"
#include "sb/ShaderBindingFactory.hpp"

namespace tr::gfx {

   Renderer::Renderer(std::shared_ptr<IGraphicsDevice> graphicsDevice) {
      const auto config = sb::Config{
          .useDescriptorBuffers = true,
          .maxTextures = 16,
      };

      layoutFactory = std::make_shared<sb::LayoutFactory>(graphicsDevice, config);

      shaderBindingFactory =
          std::make_shared<sb::ShaderBindingFactory>(graphicsDevice,
                                                     layoutFactory,
                                                     config.useDescriptorBuffers);
   }

   void Renderer::render() {
   }

   void Renderer::waitIdle() {
   }
}