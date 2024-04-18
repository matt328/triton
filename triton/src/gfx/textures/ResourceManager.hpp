#pragma once

#include "gfx/Handles.hpp"
#include "gfx/ObjectData.hpp"
#include "gfx/geometry/Mesh.hpp"
#include "gfx/geometry/Vertex.hpp"
#include "gfx/textures/Texture.hpp"

#include "util/TaskQueue.hpp"

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

   struct RenderData {
      CameraData cameraData;
      std::vector<ObjectData> objectData;
      std::vector<MeshHandle> meshHandles;
   };

   /*
      ResourceManager provides the sync point between game world and renderer.
      2 types of things need synced, Descriptor Set bindings, and buffer contents.
      DS Bindings - texture DS will have texture image infos added/removed
         - this will eventually need kept insync with the material buffer but for now textures are
      referenced from the ObjectData buffer
      Buffers - right now PerFrame and ObjectData
         - PerFrame is only current camera data - calculated by the CameraSystem
         - ObjectData is every object that needs rendered, and contains mesh handle, texture handle,
      and transformation matrix and is calculated by the RenderSystem

      I think this data will need to be ~copied~ std::moved into synchronized structures so that the
      renderer can lock and read a coherent copy of it while the GameWorld system might tick again
      and produce a new copy of the data during a read. So we need a copy and not a reference to the
      data or else the data could get changed while the renderer is working with it.

      Data can actually be std::move()'d into the synced render data struct. This will trash the
      existing temporaries from the systems, so we'll just have to reinit structs and vectors but
      that's actually what I want to happen.
   */
   class ResourceManager {
    public:
      ResourceManager(const GraphicsDevice& graphicsDevice);
      ~ResourceManager();

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      ResourceManager& operator=(const ResourceManager&) = delete;
      ResourceManager& operator=(ResourceManager&&) = delete;

      std::future<ModelHandle> loadModelAsync(const std::filesystem::path& filename);
      ModelHandle loadModelInt(const std::filesystem::path& filename);

      std::unique_ptr<Geometry::Mesh<Geometry::Vertex, uint32_t>>& getMesh(MeshHandle meshHandle) {
         return meshList.at(meshHandle);
      }

      void accessTextures(
          std::function<void(const std::vector<vk::DescriptorImageInfo>&)> fn) const;

    private:
      const GraphicsDevice& graphicsDevice;
      std::unique_ptr<util::TaskQueue> taskQueue;
      std::unique_ptr<AllocatedBuffer> stagingBuffer;

      std::vector<std::unique_ptr<Geometry::Mesh<Geometry::Vertex, uint32_t>>> meshList;

      mutable TracyLockable(std::mutex, textureListMutex);
      std::vector<vk::DescriptorImageInfo> textureInfoList;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      MeshHandle createMesh(const tinygltf::Model&, const tinygltf::Primitive& primitive);
      TextureHandle createTexture(const tinygltf::Model& model, std::size_t textureIndex);
   };
}
