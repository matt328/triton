#pragma once

#include "../vma_raii.hpp"

namespace vk {
   enum class Format;
}

namespace vk::raii {
   class Device;
   class Allocator;
}

namespace tr::gfx {
   class ImmediateContext;
}

namespace tr::gfx::Textures {
   class Texture;

   class TextureFactory {
    public:
      TextureFactory(const Allocator& raiillocator,
                     const vk::raii::Device& device,
                     const ImmediateContext& graphicsContext,
                     const ImmediateContext& transferContext);

      [[nodiscard]] std::unique_ptr<Texture> createTexture2D(
          const std::string_view& filename) const;

      static std::unique_ptr<Texture> createTexture2DFromBuffer(void* buffer,
                                                                vk::DeviceSize bufferSize,
                                                                vk::Format format,
                                                                uint32_t texWidth,
                                                                uint32_t texHeight);

    private:
      const Allocator& raiillocator;
      const vk::raii::Device& device;
      const ImmediateContext& graphicsContext;
      const ImmediateContext& transferContext;
   };
}