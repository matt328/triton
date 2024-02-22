#include "TextureFactory.hpp"
#include "Texture.hpp"

namespace Triton::Graphics::Textures {

   TextureFactory::TextureFactory(const Allocator& raiillocator,
                                  const vk::raii::Device& device,
                                  const ImmediateContext& graphicsContext,
                                  const ImmediateContext& transferContext)
       : raiillocator(raiillocator),
         device(device),
         graphicsContext(graphicsContext),
         transferContext(transferContext) {
   }

   std::unique_ptr<Texture> TextureFactory::createTexture2D(
       const std::string_view& filename) const {
      return std::make_unique<Texture>(filename,
                                       raiillocator,
                                       device,
                                       graphicsContext,
                                       transferContext);
   }

   std::unique_ptr<Texture> TextureFactory::createTexture2DFromBuffer(
       [[maybe_unused]] void* buffer,
       [[maybe_unused]] vk::DeviceSize bufferSize,
       [[maybe_unused]] vk::Format format,
       [[maybe_unused]] uint32_t texWidth,
       [[maybe_unused]] uint32_t texHeight) {
      // TODO Use or get rid of this
      return nullptr;
   }
}