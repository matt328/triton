#pragma once

#include "gfx/Handles.hpp"
#include "gfx/geometry/Mesh.hpp"
#include "gfx/geometry/Vertex.hpp"
#include "gfx/textures/Texture.hpp"

namespace tr::gfx {
   class VkContext;
   class GraphicsDevice;
   class Allocator;
   class AllocatedBuffer;
   class AllocatedImage;
}

namespace tr::gfx::tx {

   constexpr uint32_t MaxImageSize = 1024 * 1024 * 8;

   struct TextureInfo {
      std::unique_ptr<AllocatedImage> image;
      std::unique_ptr<vk::raii::ImageView> imageView;
      std::unique_ptr<vk::raii::Sampler> sampler;
      vk::ImageLayout imageLayout;
   };

   constexpr auto DefaultSamplerInfo =
       vk::SamplerCreateInfo{.magFilter = vk::Filter::eLinear,
                             .minFilter = vk::Filter::eLinear,
                             .mipmapMode = vk::SamplerMipmapMode::eLinear,
                             .addressModeU = vk::SamplerAddressMode::eRepeat,
                             .addressModeV = vk::SamplerAddressMode::eRepeat,
                             .addressModeW = vk::SamplerAddressMode::eRepeat,
                             .mipLodBias = 0.f,
                             .anisotropyEnable = VK_TRUE,
                             .maxAnisotropy = 1, // TODO: look this up
                             .compareEnable = VK_FALSE,
                             .compareOp = vk::CompareOp::eAlways,
                             .minLod = 0.f,
                             .maxLod = 0.f,
                             .borderColor = vk::BorderColor::eIntOpaqueBlack,
                             .unnormalizedCoordinates = VK_FALSE};

   class ResourceManager {
    public:
      ResourceManager(const GraphicsDevice& graphicsDevice);
      ~ResourceManager();

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      ResourceManager& operator=(const ResourceManager&) = delete;
      ResourceManager& operator=(ResourceManager&&) = delete;

      ModelHandle loadModel(const std::filesystem::path& filename);

      std::unique_ptr<Geometry::Mesh<Geometry::Vertex, uint32_t>>& getMesh(MeshHandle meshHandle) {
         return meshList.at(meshHandle);
      }

      std::unique_ptr<Textures::Texture>& getTexture(TextureHandle textureHandle) {
         return textureList.at(textureHandle);
      }

    private:
      const GraphicsDevice& graphicsDevice;
      std::unique_ptr<AllocatedBuffer> stagingBuffer;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;
      std::vector<std::unique_ptr<Geometry::Mesh<Geometry::Vertex, uint32_t>>> meshList;

      MeshHandle createMesh(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive);
      TextureHandle createTexture(const fastgltf::Asset& asset,
                                  std::size_t textureIndex,
                                  const std::filesystem::path& folder);
   };
}