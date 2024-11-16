#pragma once

#include "IShaderBindingFactory.hpp"
#include "gfx/IRenderContext.hpp"

namespace tr::gfx {
   class IGraphicsDevice;
   namespace sb {
      class LayoutFactory;
   }
}

namespace tr::gfx::sb {

   class ShaderBindingFactory : public IShaderBindingFactory {
    public:
      ShaderBindingFactory(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                           std::shared_ptr<LayoutFactory> layoutFactory,
                           RenderContextConfig rendererConfig);
      ~ShaderBindingFactory();

      ShaderBindingFactory(const ShaderBindingFactory&) = delete;
      auto operator=(const ShaderBindingFactory&) -> ShaderBindingFactory& = delete;

      ShaderBindingFactory(ShaderBindingFactory&&) = delete;
      auto operator=(ShaderBindingFactory&&) -> ShaderBindingFactory& = delete;

      /// Allocates a new ShaderBinding
      [[nodiscard]] auto createShaderBinding(ShaderBindingHandle handle) const
          -> std::unique_ptr<ShaderBinding> override;

    private:
      RenderContextConfig config;
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<LayoutFactory> layoutFactory;

      std::unique_ptr<vk::raii::DescriptorPool> permanentPool;
   };
}