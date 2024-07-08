#pragma once

#include "cm/Handles.hpp"
#include "cm/RenderData.hpp"

#include "gfx/geometry/Mesh.hpp"
#include "gfx/textures/Texture.hpp"
#include "gfx/geometry/GeometryHandles.hpp"

#include "util/TaskQueue.hpp"

namespace tr::gfx::mem {
   class Allocator;
   class Buffer;
   class Image;
}

namespace tr::gfx {
   class VkContext;
   class GraphicsDevice;
}

namespace tr::gfx::geo {
   class GeometryData;
   class GeometryFactory;
}

namespace tr::gfx::tx {

   constexpr uint32_t MaxImageSize = 1024 * 1024 * 8;

   struct TextureInfo {
      std::unique_ptr<mem::Image> image;
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

      auto createTerrain() -> std::future<cm::ModelHandle>;
      auto createTerrainInt() -> cm::ModelHandle;

      std::future<cm::ModelHandle> loadModelAsync(const std::filesystem::path& filename);
      cm::ModelHandle loadModelInt(const std::filesystem::path& filename);

      std::future<cm::LoadedSkinnedModelData> loadSkinnedModelAsync(
          const std::filesystem::path& modelPath,
          const std::filesystem::path& skeletonPath,
          const std::filesystem::path& animationPath);
      cm::LoadedSkinnedModelData loadSkinnedModelInt(const std::filesystem::path& modelPath,
                                                     const std::filesystem::path& skeletonPath,
                                                     const std::filesystem::path& animationPath);

      auto createStaticMesh(const geo::GeometryData& geometry) -> cm::MeshHandle;

      auto& getMesh(cm::MeshHandle meshHandle) {
         return meshList.at(meshHandle);
      }

      void accessTextures(
          std::function<void(const std::vector<vk::DescriptorImageInfo>&)> fn) const;

      void setRenderData(cm::RenderData& newRenderData);
      void accessRenderData(std::function<void(cm::RenderData&)> fn);

    private:
      const GraphicsDevice& graphicsDevice;

      std::unique_ptr<geo::GeometryFactory> geometryFactory;
      std::unique_ptr<util::TaskQueue> taskQueue;
      std::vector<geo::Mesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      mutable TracyLockable(std::mutex, renderDataMutex);
      cm::RenderData renderData;

      auto uploadGeometry(const geo::TexturedGeometryHandle& handles) -> cm::ModelHandle;
      auto uploadSkinnedGeometry(const geo::SkinnedGeometryData& sgd) -> cm::LoadedSkinnedModelData;

      cm::MeshHandle createMesh(const tinygltf::Model&, const tinygltf::Primitive& primitive);
      cm::TextureHandle createTexture(const tinygltf::Model& model, std::size_t textureIndex);
   };
}
