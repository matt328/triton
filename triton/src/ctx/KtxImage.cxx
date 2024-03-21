#include "ctx/KtxImage.hpp"
#include <ktx.h>

namespace tr::util {

   KtxImage::KtxImage() {
      const auto whitePixel = std::array<ktx_uint8_t, 4>{255, 255, 255, 255};
      if (ktxTexture_CreateFromMemory(
              whitePixel.data(),
              sizeof(whitePixel),
              ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
              &texture) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to create white texture");
      }
   }

   KtxImage::KtxImage(const std::filesystem::path& filename) {
      if (ktxTexture_CreateFromNamedFile(filename.string().c_str(),
                                         KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                                         &texture) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }
   }

   KtxImage::KtxImage(const void* data, size_t size) {
      auto result = ktxTexture_CreateFromMemory((ktx_uint8_t*)data, size, 0, &texture);
      if (result != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }
   }

   KtxImage::~KtxImage() {
      ktxTexture_Destroy(texture);
   }

}