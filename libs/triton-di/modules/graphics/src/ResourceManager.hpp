#pragma once

#include "BaseException.hpp"

#include "cm/Handles.hpp"
#include "cm/LockableResource.hpp"

#include "geo/Mesh.hpp"
#include "tex/Texture.hpp"
#include "geo/GeometryHandles.hpp"
#include <optional>

namespace tr::gfx::mem {
   class Allocator;
   class Buffer;
   class Image;
}

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::geo {
   class GeometryData;
   class GeometryFactory;
}

namespace tr::gfx {

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

   class ResourceManager {
    public:
      explicit ResourceManager(std::shared_ptr<IGraphicsDevice> newGraphicsDevice);
      ~ResourceManager();

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      auto operator=(const ResourceManager&) -> ResourceManager& = delete;
      auto operator=(ResourceManager&&) -> ResourceManager& = delete;

      auto createTerrain() -> std::vector<cm::ModelData>;

      auto createModel(const std::filesystem::path& filename) noexcept -> cm::ModelData;

      auto createAABB(const glm::vec3& min, const glm::vec3& max) noexcept -> cm::ModelData;

      auto createStaticMesh(const geo::GeometryData& geometry) -> cm::MeshHandle;

      auto getMesh(const cm::MeshHandle meshHandle) -> auto& {
         return meshList.at(meshHandle);
      }

      auto getTextures() const -> cm::LockableResource<const std::vector<vk::DescriptorImageInfo>>;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;

      std::unique_ptr<geo::GeometryFactory> geometryFactory;
      std::vector<geo::ImmutableMesh> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<tex::Texture>> textureList;

      /// Uploads Geometry (and images) to the GPU
      /// @throws ResourceUploadException if there's an error uploading.
      auto uploadGeometry(const geo::GeometryHandle& geometryHandle,
                          cm::Topology topology = cm::Topology::Triangles,
                          std::optional<geo::ImageHandle> imageHandle = std::nullopt)
          -> cm::ModelData;
   };
}
