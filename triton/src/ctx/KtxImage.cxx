#include "ctx/KtxImage.hpp"
#include <ktx.h>

namespace tr::util {

   KtxImage::KtxImage() : name{"white"} {
      const auto whitePixel = std::array<ktx_uint8_t, 4>{255, 255, 255, 255};
      if (ktxTexture_CreateFromMemory(whitePixel.data(),
                                      sizeof(whitePixel),
                                      ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_NO_FLAGS,
                                      &texture) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to create white texture");
      }
   }

   KtxImage::KtxImage(const std::filesystem::path& filename) : name{filename.filename().string()} {
      if (ktxTexture_CreateFromNamedFile(
              filename.string().c_str(),
              ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_SKIP_KVDATA_BIT,
              &texture) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }
   }

   KtxImage::KtxImage(const ktx_uint8_t* data, size_t size) : name{"unnamed texture"} {
      ktxTexture* texture2;
      if (ktxTexture_CreateFromNamedFile(
              R"(C:\Users\Matt\Projects\game-assets\models\quarter_alb.ktx)",
              ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_NO_FLAGS,
              &texture2) != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }

      auto result =
          ktxTexture_CreateFromMemory(data,
                                      size,
                                      ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_NO_FLAGS,
                                      &texture);
      if (result != KTX_SUCCESS) {
         throw std::runtime_error("Failed to load KTX File");
      }
      auto image = texture->pData;
      Log::debug << "finished loading texture" << std::endl;
   }

   KtxImage::~KtxImage() {
      ktxTexture_Destroy(texture);
   }

}