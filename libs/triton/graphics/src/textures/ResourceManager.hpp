#pragma once

#include "cm/Handles.hpp"
#include "cm/RenderData.hpp"

#include "futures/future.hpp"
#include "geometry/Mesh.hpp"
#include "textures/Texture.hpp"
#include "geometry/GeometryHandles.hpp"

#include "TaskQueue.hpp"

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

   class ResourceUploadException : public std::runtime_error {
    public:
      explicit ResourceUploadException(const std::string& message) : std::runtime_error(message) {
      }
   };

   class ResourceCreateException : public std::runtime_error {
    public:
      explicit ResourceCreateException(const std::string& message) : std::runtime_error(message) {
      }
   };

   class ResourceManager {
    public:
      ResourceManager(const GraphicsDevice& graphicsDevice);
      ~ResourceManager();

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      ResourceManager& operator=(const ResourceManager&) = delete;
      ResourceManager& operator=(ResourceManager&&) = delete;

      auto createTerrain(const uint32_t size) -> futures::cfuture<cm::ModelHandle>;

      auto createModel(const std::filesystem::path& filename)
          -> futures::cfuture<std::optional<cm::ModelHandle>>;

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
      std::vector<geo::ImmutableMesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      mutable TracyLockable(std::mutex, renderDataMutex);
      cm::RenderData renderData;

      /// Uploads Geometry (and images) to the GPU
      /// @throws ResourceUploadException if there's an error uploading.
      auto uploadGeometry(const geo::GeometryHandle& geometryHandle,
                          const geo::ImageHandle& imageHandle) -> cm::ModelHandle;
   };
}
