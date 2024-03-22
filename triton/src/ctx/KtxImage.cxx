#include "ctx/KtxImage.hpp"
#include <ktx.h>

namespace tr::util {

   KtxImage::KtxImage() {
      const auto whitePixel = std::array<ktx_uint8_t, 4>{255, 255, 255, 255};
      if (ktxTexture2_CreateFromMemory(whitePixel.data(),
                                       sizeof(whitePixel),
                                       ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_NO_FLAGS,
                                       &texture) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to create white texture");
      }
   }

   KtxImage::KtxImage(const std::filesystem::path& filename) {
      if (ktxTexture2_CreateFromNamedFile(
              filename.string().c_str(),
              ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_SKIP_KVDATA_BIT,
              &texture) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }
   }

   // For some reason when telling fastgltf to load images, it trashes them up
   // so just tell it not to and try to get the uri thing to work instead.
   KtxImage::KtxImage(const ktx_uint8_t* data, size_t size) {
      auto filename =
          std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets\models\quarter_alb.ktx)"};
      auto result = ktxTexture2_CreateFromNamedFile(
          filename.string().c_str(),
          ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
          &texture);
      if (result != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }
   }

   KtxImage::~KtxImage() {
      // ktxTexture2_Destroy(texture);
   }

}