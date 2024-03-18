#pragma once

namespace tr::util {
   class KtxImage {
    public:
      KtxImage();
      KtxImage(const std::filesystem::path& filename);
      KtxImage(const void* data, size_t size);

      ~KtxImage();

      KtxImage(const KtxImage&) = delete;
      KtxImage(KtxImage&&) = default;
      KtxImage& operator=(const KtxImage&) = delete;
      KtxImage& operator=(KtxImage&&) = default;

      [[nodiscard]] ktxTexture* get() const {
         return texture;
      }

    private:
      ktxTexture* texture{};
   };
}