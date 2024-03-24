#pragma once

namespace tr::util {
   class KtxImage {
    public:
      KtxImage();
      KtxImage(const std::filesystem::path& filename);
      KtxImage(const ktx_uint8_t* data, size_t size);

      ~KtxImage();

      KtxImage(const KtxImage&) = delete;
      KtxImage(KtxImage&&) = default;
      KtxImage& operator=(const KtxImage&) = delete;
      KtxImage& operator=(KtxImage&&) = default;

      [[nodiscard]] ktxTexture* getTexture() const {
         return texture;
      }

      [[nodiscard]] const std::string& getName() const {
         return name;
      }

    private:
      std::string name;
      ktxTexture* texture = nullptr;
   };
}