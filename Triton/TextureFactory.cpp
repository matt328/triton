#include "TextureFactory.h"

#include "Texture.h"

TextureFactory::TextureFactory(const vma::raii::Allocator& raiillocator,
                               const vk::raii::Device& device,
                               const ImmediateContext& graphicsContext,
                               const ImmediateContext& transferContext)
    : raiillocator(raiillocator)
    , device(device)
    , graphicsContext(graphicsContext)
    , transferContext(transferContext) {
}

std::unique_ptr<Texture> TextureFactory::createTexture2D(const std::string_view& filename) const {
   return std::make_unique<Texture>(
       filename, raiillocator, device, graphicsContext, transferContext);
}

std::unique_ptr<Texture> TextureFactory::createTexture2DFromBuffer(void* buffer,
                                                                   vk::DeviceSize bufferSize,
                                                                   vk::Format format,
                                                                   uint32_t texWidth,
                                                                   uint32_t texHeight) {
   return nullptr;
}
