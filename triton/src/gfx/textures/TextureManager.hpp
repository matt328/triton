#pragma once

#include "ctx/KtxImage.hpp"
#include "gfx/Handles.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx {
   class VkContext;
   class GraphicsDevice;
   class Allocator;
   class AllocatedBuffer;
   class AllocatedImage;
}

namespace tr::gfx::tx {

   using TransitionBarrierInfo =
       std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlagBits, vk::PipelineStageFlagBits>;

   constexpr uint32_t MaxImageSize = 1024 * 1024 * 8;

   struct ImageInfo {
      std::unique_ptr<AllocatedImage> image;
      std::unique_ptr<vk::raii::ImageView> imageView;
      std::unique_ptr<vk::raii::Sampler> sampler;
      std::optional<vk::ImageSubresourceRange> subresourceRange;
      vk::ImageLayout imageLayout;
   };

   class TextureManager {
    public:
      TextureManager(const GraphicsDevice& graphicsDevice);
      ~TextureManager();

      TextureManager(const TextureManager&) = delete;
      TextureManager(TextureManager&&) = delete;
      TextureManager& operator=(const TextureManager&) = delete;
      TextureManager& operator=(TextureManager&&) = delete;

      std::vector<TextureHandle> uploadTextures(
          const std::vector<util::KtxImage>& ktxImages,
          const std::vector<vk::SamplerCreateInfo>& samplerInfo);

    private:
      const GraphicsDevice& graphicsDevice;

      std::unique_ptr<AllocatedBuffer> stagingBuffer;

      std::vector<ImageInfo> imageInfoList;

      const TransitionBarrierInfo createTransitionBarrier(
          const vk::Image& image,
          const vk::ImageLayout oldLayout,
          const vk::ImageLayout newLayout,
          const vk::ImageSubresourceRange subresourceRange);
   };
}