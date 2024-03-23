#pragma once

#include "ctx/KtxImage.hpp"
#include "gfx/Handles.hpp"

namespace tr::gfx {
   class VkContext;
   class Allocator;
   class AllocatedBuffer;
}

namespace tr::gfx::tx {

   constexpr uint32_t MaxImageSize = 1024 * 1024 * 8;

   struct ImageInfo {
      std::unique_ptr<vk::raii::Image> image;
      std::unique_ptr<vk::raii::ImageView> imageView;
      std::unique_ptr<vk::raii::Sampler> sampler;
      vk::ImageLayout imageLayout;
   };

   class TextureManager {
    public:
      TextureManager(const VkContext& context, const Allocator& allocator);
      ~TextureManager();

      TextureManager(const TextureManager&) = delete;
      TextureManager(TextureManager&&) = delete;
      TextureManager& operator=(const TextureManager&) = delete;
      TextureManager& operator=(TextureManager&&) = delete;

      std::vector<TextureHandle> uploadTextures(const std::vector<util::KtxImage>& ktxImages);

    private:
      const VkContext& context;
      const Allocator& allocator;

      std::unique_ptr<AllocatedBuffer> stagingBuffer;

      std::vector<ImageInfo> imageInfoList;
   };
}