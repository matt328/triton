#include "textures/ResourceManager.hpp"

#include "cm/Handles.hpp"
#include "cm/RenderData.hpp"

#include "as/Model.hpp"

#include "HeightField.hpp"

#include "geometry/GeometryFactory.hpp"
#include "geometry/GeometryHandles.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

#include "GraphicsDevice.hpp"
#include "VkContext.hpp"

#include "geometry/GeometryData.hpp"
#include "geometry/Mesh.hpp"

#include "mem/Allocator.hpp"
#include "mem/Buffer.hpp"

namespace tr::gfx::tx {
   ResourceManager::ResourceManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {
      taskQueue = std::make_unique<util::TaskQueue>();
      geometryFactory = std::make_unique<geo::GeometryFactory>();
   }

   ResourceManager::~ResourceManager() {
   }

   void ResourceManager::setRenderData(cm::RenderData& newRenderData) {
      std::lock_guard<LockableBase(std::mutex)> lock(renderDataMutex);
      LockableName(renderDataMutex, "SetRenderData", 13);
      LockMark(renderDataMutex);
      renderData = newRenderData;
   }

   void ResourceManager::accessRenderData(std::function<void(cm::RenderData&)> fn) {
      std::lock_guard<LockableBase(std::mutex)> lock(renderDataMutex);
      LockableName(renderDataMutex, "AccessRenderData", 16);
      LockMark(renderDataMutex);
      fn(renderData);
   }

   auto ResourceManager::createTerrain(const uint32_t size) -> futures::cfuture<cm::ModelHandle> {
      ZoneNamedN(n, "ResourceManager::createTerrain", true);

      const auto createFn = [this, size]() {
         ZoneNamedN(z, "Creating Terrain", true);
         const auto heightfield = ct::HeightField{static_cast<int>(size)};

         // change this to return
         const auto dataHandle = geometryFactory->createGeometryFromHeightfield(heightfield);

         const auto pr = dataHandle.begin();

         const auto modelHandle = uploadGeometry(pr->first, pr->second);
         geometryFactory->unload(dataHandle);
         return modelHandle;
      };

      return futures::async(createFn);
   }

   auto ResourceManager::createModel(const std::filesystem::path& filename)
       -> futures::cfuture<std::optional<cm::ModelHandle>> {
      ZoneNamedN(n, "ResourceManager::loadModel", true);

      const auto createFn = [this, filename]() -> std::optional<cm::ModelHandle> {
         ZoneNamedN(z, "Loading Model", true);

         const auto modelData = geometryFactory->loadTrm(filename);

         if (!modelData) {
            return std::nullopt;
         }

         const auto& md = *modelData;

         const auto modelHandle = uploadGeometry(md.getGeometryHandle(), md.getImageHandle());

         // geometryFactory->unload(tgh);

         return modelHandle;
      };

      return futures::async(createFn);
   }

   /// Uploads the Geometry and Texture data and creates space in a buffer for the skeleton's
   /// joint matrix data
   auto ResourceManager::uploadSkinnedGeometry(const geo::SkinnedGeometryData& sgd)
       -> std::optional<cm::LoadedSkinnedModelData> {
      const auto modelHandle = uploadGeometry(sgd.geometryHandle, sgd.imageHandle);

      if (!modelHandle) {
         Log.warn("Geometry was not uploaded");
         return std::nullopt;
      }

      const auto handle = *modelHandle;

      const auto meshHandle = handle.begin()->first;
      const auto textureHandle = handle.begin()->second;
      const auto smh = cm::LoadedSkinnedModelData{.meshHandle = meshHandle,
                                                  .textureHandle = textureHandle,
                                                  .skeletonHandle = sgd.skeletonHandle,
                                                  .animationHandle = sgd.animationHandle};

      return smh;
   }

   auto ResourceManager::uploadGeometry(const geo::GeometryHandle& geometryHandle,
                                        const geo::ImageHandle& imageHandle)
       -> std::optional<cm::ModelHandle> {
      auto& allocator = graphicsDevice.getAllocator();
      auto& context = graphicsDevice.getAsyncTransferContext();

      const auto geometryData = geometryFactory->getGeometryData(geometryHandle);

      // Prepare Vertex Buffer
      const auto vbSize = geometryData.vertexDataSize();
      const auto vbStagingBuffer = allocator.createStagingBuffer(vbSize, "Vertex Staging Buffer");

      void* vbData = allocator.mapMemory(*vbStagingBuffer);
      memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
      allocator.unmapMemory(*vbStagingBuffer);

      const auto ibSize = geometryData.indexDataSize();

      auto vertexBuffer = allocator.createGpuVertexBuffer(vbSize, "GPU Vertex");
      auto indexBuffer = allocator.createGpuIndexBuffer(ibSize, "GPU Index");
      const auto indicesCount = geometryData.indices.size();

      // Prepare Index Buffer
      const auto ibStagingBuffer = allocator.createStagingBuffer(ibSize, "Index Staging Buffer");

      auto data = allocator.mapMemory(*ibStagingBuffer);
      memcpy(data, geometryData.indices.data(), static_cast<size_t>(ibSize));
      allocator.unmapMemory(*ibStagingBuffer);

      // Upload Buffers
      context.submit([&](const vk::raii::CommandBuffer& cmd) {
         const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = vbSize};
         cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer->getBuffer(), vbCopy);
         const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = ibSize};
         cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
      });

      const auto image = geometryFactory->getImageData(imageHandle);
      const auto textureHandle = textureList.size();
      textureList.emplace_back(std::make_unique<Textures::Texture>((void*)image.data.data(),
                                                                   image.width,
                                                                   image.height,
                                                                   image.component,
                                                                   allocator,
                                                                   graphicsDevice.getVulkanDevice(),
                                                                   context));

      { // Only need to guard access to the textureInfoList
         ZoneNamedN(c, "Update TextureInfoList", true);
         std::lock_guard<LockableBase(std::mutex)> lock(textureListMutex);
         LockMark(textureListMutex);
         LockableName(textureListMutex, "Mutate", 6);
         textureInfoList.emplace_back(textureList[textureHandle]->getImageInfo());
      }

      const auto meshHandle = meshList.size();
      meshList.emplace_back(std::move(vertexBuffer), std::move(indexBuffer), indicesCount);

      return cm::ModelHandle{meshHandle, textureHandle};
   }

   void ResourceManager::accessTextures(
       std::function<void(const std::vector<vk::DescriptorImageInfo>&)> fn) const {
      if (textureInfoList.empty()) {
         return;
      }
      std::lock_guard<LockableBase(std::mutex)> lock(textureListMutex);
      LockableName(textureListMutex, "Access", 6);
      LockMark(textureListMutex);
      fn(textureInfoList);
   }
}