#pragma once

namespace tr::gfx {
   class IGraphicsDevice;
   namespace sb {
      class LayoutFactory;
   }
}

namespace tr::gfx::sb {

   class ShaderBinding;

   enum class ShaderBindingHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
      AnimationData = 4,
      // TODO(Matt): Add an additional one here for DebugGroup
   };

   class ShaderBindingFactory {
    public:
      ShaderBindingFactory(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                           std::shared_ptr<LayoutFactory> layoutFactory,
                           bool useDescriptorBuffers = false);
      ~ShaderBindingFactory();

      ShaderBindingFactory(const ShaderBindingFactory&) = delete;
      auto operator=(const ShaderBindingFactory&) -> ShaderBindingFactory& = delete;

      ShaderBindingFactory(ShaderBindingFactory&&) = delete;
      auto operator=(ShaderBindingFactory&&) -> ShaderBindingFactory& = delete;

      /// Allocates a new ShaderBinding
      [[nodiscard]] auto createShaderBinding(ShaderBindingHandle handle) const
          -> std::unique_ptr<ShaderBinding>;

      [[nodiscard]] auto getLayoutFactory() const -> auto& {
         return *layoutFactory;
      }

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<LayoutFactory> layoutFactory;
      bool useDescriptorBuffers{};

      std::unique_ptr<vk::raii::DescriptorPool> permanentPool;
   };
}