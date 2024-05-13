#pragma once

#include "gfx/Handles.hpp"
#include "gfx/RenderData.hpp"
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

      auto createTerrain() -> std::future<ModelHandle>;
      auto createTerrainInt() -> ModelHandle;

      std::future<ModelHandle> loadModelAsync(const std::filesystem::path& filename);
      ModelHandle loadModelInt(const std::filesystem::path& filename);

      std::future<SkinnedModelHandle> loadSkinnedModelAsync(const std::string_view modelPath,
                                                            const std::string_view skeletonPath,
                                                            const std::string_view animationPath);
      SkinnedModelHandle loadSkinnedModelInt(const std::string_view modelPath,
                                             const std::string_view skeletonPath,
                                             const std::string_view animationPath);

      auto createStaticMesh(const geo::GeometryData& geometry) -> MeshHandle;

      auto& getMesh(MeshHandle meshHandle) {
         return meshList.at(meshHandle);
      }

      void accessTextures(
          std::function<void(const std::vector<vk::DescriptorImageInfo>&)> fn) const;

      void setRenderData(RenderData& newRenderData);
      void accessRenderData(std::function<void(RenderData&)> fn);

    private:
      const GraphicsDevice& graphicsDevice;

      std::unique_ptr<geo::GeometryFactory> geometryFactory;
      std::unique_ptr<util::TaskQueue> taskQueue;
      std::vector<geo::Mesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      mutable TracyLockable(std::mutex, renderDataMutex);
      RenderData renderData;

      auto uploadGeometry(const geo::TexturedGeometryHandle& handles) -> ModelHandle;

      MeshHandle createMesh(const tinygltf::Model&, const tinygltf::Primitive& primitive);
      TextureHandle createTexture(const tinygltf::Model& model, std::size_t textureIndex);
   };
}
