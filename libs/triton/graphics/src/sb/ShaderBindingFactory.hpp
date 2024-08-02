#pragma once

namespace tr::gfx {
   class GraphicsDevice;
   namespace ds {
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
   };

   class ShaderBindingFactory {
    public:
      ShaderBindingFactory(const GraphicsDevice& graphicsDevice,
                           const ds::LayoutFactory& layoutFactory,
                           bool useDescriptorBuffers = false);
      ~ShaderBindingFactory();

      ShaderBindingFactory(const ShaderBindingFactory&) = delete;
      ShaderBindingFactory& operator=(const ShaderBindingFactory&) = delete;

      ShaderBindingFactory(ShaderBindingFactory&&) = delete;
      ShaderBindingFactory& operator=(ShaderBindingFactory&&) = delete;

      /// Allocates a new ShaderBinding
      [[nodiscard]] auto createShaderBinding(ShaderBindingHandle handle) const
          -> std::unique_ptr<ShaderBinding>;

      [[nodiscard]] auto& getLayoutFactory() const {
         return layoutFactory;
      }

    private:
      const GraphicsDevice& graphicsDevice;
      const ds::LayoutFactory& layoutFactory;
      bool useDescriptorBuffers{};

      std::unique_ptr<vk::raii::DescriptorPool> permanentPool;
   };
}