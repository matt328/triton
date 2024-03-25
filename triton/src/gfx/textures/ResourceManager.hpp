#pragma once

#include "gfx/Handles.hpp"
#include <fastgltf/types.hpp>

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

   struct TextureInfo {
      std::unique_ptr<AllocatedImage> image;
      std::unique_ptr<vk::raii::ImageView> imageView;
      std::unique_ptr<vk::raii::Sampler> sampler;
      vk::ImageLayout imageLayout;
   };

   class ResourceManager {
    public:
      ResourceManager(const GraphicsDevice& graphicsDevice);
      ~ResourceManager();

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      ResourceManager& operator=(const ResourceManager&) = delete;
      ResourceManager& operator=(ResourceManager&&) = delete;

      ModelHandle loadModel(const std::filesystem::path& filename);

      // std::vector<TextureHandle> uploadTextures(
      //     const std::vector<util::KtxImage>& ktxImages,
      //     const std::vector<vk::SamplerCreateInfo>& samplerInfo);

    private:
      const GraphicsDevice& graphicsDevice;
      std::unique_ptr<AllocatedBuffer> stagingBuffer;
      std::vector<TextureInfo> textureList;

      std::vector<TextureHandle> uploadImages(const fastgltf::Asset& asset,
                                              const std::filesystem::path& path);
      std::vector<MeshHandle> uploadMeshes(const fastgltf::Asset& asset);

      ModelHandle createModelHandles(const fastgltf::Asset& asset,
                                     const std::vector<TextureHandle>& textureHandles,
                                     const std::vector<MeshHandle>& meshHandles);

      const TransitionBarrierInfo createTransitionBarrier(
          const vk::Image& image,
          const vk::ImageLayout oldLayout,
          const vk::ImageLayout newLayout,
          const vk::ImageSubresourceRange subresourceRange);
   };
}