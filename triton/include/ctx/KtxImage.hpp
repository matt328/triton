#pragma once

namespace tr::util {
   class KtxImage {
    public:
      KtxImage();
      KtxImage(const std::filesystem::path& filename);
      KtxImage(const ktx_uint8_t* data, size_t size);

      ~KtxImage();

      KtxImage(const KtxImage&) = delete;
      KtxImage(KtxImage&&) = delete;
      KtxImage& operator=(const KtxImage&) = delete;
      KtxImage& operator=(KtxImage&&) = delete;

      [[nodiscard]] ktxTexture2* getTexture() const {
         return texture;
      }

      [[nodiscard]] const std::string& getName() const {
         return name;
      }

    private:
      const std::string name;
      ktxTexture2* texture{};
   };
}