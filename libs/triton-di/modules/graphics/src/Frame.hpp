#pragma once

namespace tr::gfx {

   class IGraphicsDevice;
   namespace sb {
      class IShaderBindingFactory;
   }

   class Frame {
    public:
      Frame(std::shared_ptr<IGraphicsDevice> graphicsDevice,
            std::shared_ptr<vk::raii::ImageView> depthImageView,
            std::shared_ptr<sb::IShaderBindingFactory> shaderBindingFactory,
            std::string_view name);
      ~Frame();

      Frame(const Frame&) = delete;
      Frame(Frame&&) = delete;
      auto operator=(const Frame&) -> Frame& = delete;
      auto operator=(Frame&&) -> Frame& = delete;

      void registerStorageBuffer(const std::string& name, size_t size);
      void destroySwapchainResources();
      void createSwapchainResources();
   };
}
