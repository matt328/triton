#pragma once

namespace tr::util {
   class KtxImage {
    public:
      KtxImage(const std::filesystem::path& filename) {
         if (ktxTexture_CreateFromNamedFile(filename.string().c_str(),
                                            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                                            &texture) != KTX_SUCCESS) {
            throw std::runtime_error("Failed to load KTX File");
         }
      }

      ~KtxImage() {
         ktxTexture_Destroy(texture);
      }

      KtxImage(const KtxImage&) = default;
      KtxImage(KtxImage&&) = delete;
      KtxImage& operator=(const KtxImage&) = default;
      KtxImage& operator=(KtxImage&&) = delete;

      [[nodiscard]] ktxTexture* get() const {
         return texture;
      }

    private:
      ktxTexture* texture{};
   };
}