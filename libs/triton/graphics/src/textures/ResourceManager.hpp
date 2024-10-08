#pragma once

#include "BaseException.hpp"

#include "cm/Handles.hpp"
#include "cm/RenderData.hpp"

#include "geometry/Mesh.hpp"
#include "textures/Texture.hpp"
#include "geometry/GeometryHandles.hpp"

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
                             .maxAnisotropy = 1, // TODO(Matt): look this up
                             .compareEnable = VK_FALSE,
                             .compareOp = vk::CompareOp::eAlways,
                             .minLod = 0.f,
                             .maxLod = 0.f,
                             .borderColor = vk::BorderColor::eIntOpaqueBlack,
                             .unnormalizedCoordinates = VK_FALSE};

   /// Exception thrown when an error occurs uploading a resource to the GPU
   class ResourceUploadException final : public BaseException {
    public:
      using BaseException::BaseException;
   };

   class ResourceCreateException final : public tr::BaseException {
    public:
      using BaseException::BaseException;
   };

   class ResourceManager {
    public:
      explicit ResourceManager(const GraphicsDevice& graphicsDevice);
      ~ResourceManager();

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      ResourceManager& operator=(const ResourceManager&) = delete;
      ResourceManager& operator=(ResourceManager&&) = delete;

      auto createTerrain(uint32_t size) -> cm::ModelData;

      auto createModel(const std::filesystem::path& filename) noexcept -> cm::ModelData;

      auto createStaticMesh(const geo::GeometryData& geometry) -> cm::MeshHandle;

      auto getMesh(const cm::MeshHandle meshHandle) -> auto& {
         return meshList.at(meshHandle);
      }

      void accessTextures(
          const std::function<void(const std::vector<vk::DescriptorImageInfo>&)>& fn) const;

      void setRenderData(const cm::gpu::RenderData& newRenderData);
      void accessRenderData(const std::function<void(cm::gpu::RenderData&)>& fn);

    private:
      const GraphicsDevice& graphicsDevice;

      std::unique_ptr<geo::GeometryFactory> geometryFactory;
      std::vector<geo::ImmutableMesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      mutable TracyLockable(std::mutex, renderDataMutex);
      cm::gpu::RenderData renderData;

      /// Uploads Geometry (and images) to the GPU
      /// @throws ResourceUploadException if there's an error uploading.
      auto uploadGeometry(const geo::GeometryHandle& geometryHandle,
                          const geo::ImageHandle& imageHandle) -> cm::ModelData;
   };
} // namespace tr::gfx::tx
