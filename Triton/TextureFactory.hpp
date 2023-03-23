#pragma once

#include "RenderDevice.hpp"

namespace vk {
   enum class Format;
}

class Texture;

namespace vk {
   namespace raii {
      class Device;
   }
} // namespace vk

class ImmediateContext;

namespace vma {
   namespace raii {
      class Allocator;
   }
} // namespace vma

class TextureFactory {
 public:
   TextureFactory(const vma::raii::Allocator& raiillocator,
                  const vk::raii::Device& device,
                  const ImmediateContext& graphicsContext,
                  const ImmediateContext& transferContext);

   std::unique_ptr<Texture> createTexture2D(const std::string_view& filename) const;
   static std::unique_ptr<Texture> createTexture2DFromBuffer(void* buffer,
                                                             vk::DeviceSize bufferSize,
                                                             vk::Format format,
                                                             uint32_t texWidth,
                                                             uint32_t texHeight);

 private:
   const vma::raii::Allocator& raiillocator;
   const vk::raii::Device& device;
   const ImmediateContext& graphicsContext;
   const ImmediateContext& transferContext;
};
