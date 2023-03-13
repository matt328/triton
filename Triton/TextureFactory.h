#pragma once
#include <memory>
#include <string_view>

class Texture;

namespace vk {
   namespace raii {
      class Device;
   }
}

class ImmediateContext;

namespace vma {
   namespace raii {
      class Allocator;
   }
}

class TextureFactory {
 public:
   TextureFactory(const vma::raii::Allocator& raiillocator,
                  const vk::raii::Device& device,
                  const ImmediateContext& graphicsContext,
                  const ImmediateContext& transferContext);

   std::unique_ptr<Texture> createTexture2D(const std::string_view& filename) const;

 private:
   const vma::raii::Allocator& raiillocator;
   const vk::raii::Device& device;
   const ImmediateContext& graphicsContext;
   const ImmediateContext& transferContext;
};
