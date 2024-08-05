#include "textures/ResourceManager.hpp"

#include "cm/Handles.hpp"
#include "cm/RenderData.hpp"

#include "as/Model.hpp"

#include "HeightField.hpp"

#include "geometry/GeometryFactory.hpp"
#include "geometry/GeometryHandles.hpp"

#include "GraphicsDevice.hpp"
#include "VkContext.hpp"

#include "geometry/GeometryData.hpp"

#include "mem/Allocator.hpp"
#include "mem/Buffer.hpp"
#include <stacktrace>

namespace tr::gfx::tx {
   ResourceManager::ResourceManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {
      taskQueue = std::make_unique<util::TaskQueue>();
      geometryFactory = std::make_unique<geo::GeometryFactory>();
   }

   ResourceManager::~ResourceManager() { // NOLINT(*-use-equals-default)
   }

   void ResourceManager::setRenderData(const cm::gpu::RenderData& newRenderData) {
      std::lock_guard lock(renderDataMutex);
      LockableName(renderDataMutex, "SetRenderData", 13);
      LockMark(renderDataMutex);
      renderData = newRenderData;
   }

   void ResourceManager::accessRenderData(const std::function<void(cm::gpu::RenderData&)>& fn) {
      std::lock_guard lock(renderDataMutex);
      LockableName(renderDataMutex, "AccessRenderData", 16);
      LockMark(renderDataMutex);
      fn(renderData);
   }

   auto ResourceManager::createTerrain(const uint32_t size) -> futures::cfuture<cm::ModelData> {
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
       -> futures::cfuture<cm::ModelData> {
      ZoneNamedN(n, "ResourceManager::loadModel", true);

      const auto createFn = [this, filename]() -> cm::ModelData {
         ZoneNamedN(z, "Loading Model", true);

         auto tritonModelData = [this, &filename]() {
            try {
               return geometryFactory->loadTrm(filename);
            } catch (const geo::IOException& ex) {
               const auto msg = fmt::format("Error loading model from file: {0}, {1}",
                                            filename.string(),
                                            ex.what());
               std::cout << std::stacktrace::current() << std::endl;
               throw ResourceCreateException(msg);
            }
         }();

         // Upload Meshes and Textures and fill out handles in modelData
         auto modelData = [this, &tritonModelData]() {
            try {
               return uploadGeometry(tritonModelData.getGeometryHandle(),
                                     tritonModelData.getImageHandle());
            } catch (const ResourceUploadException& ex) {
               const auto msg = fmt::format("Error uploading model: {0}", ex.what());
               throw ResourceCreateException(msg);
            }
         }();
         geometryFactory->unload(
             {{tritonModelData.getGeometryHandle(), tritonModelData.getImageHandle()}});

         if (auto skinData = tritonModelData.getSkinData()) {
            modelData.skinData = std::move(skinData);
         }

         return modelData;
      };

      return futures::async(createFn);
   }

   auto ResourceManager::uploadGeometry(const geo::GeometryHandle& geometryHandle,
                                        const geo::ImageHandle& imageHandle) -> cm::ModelData {
      auto& allocator = graphicsDevice.getAllocator();
      auto& context = graphicsDevice.getAsyncTransferContext();

      auto geometryData = geo::GeometryData{};
      try {
         geometryData = geometryFactory->getGeometryData(geometryHandle);
      } catch (const geo::GeometryDataNotFoundException& ex) {
         throw ResourceUploadException(fmt::format("Error getting geometry data: {0}", ex.what()));
      }

      // Prepare Vertex Buffer
      const auto vbSize = geometryData.vertexDataSize();
      const auto ibSize = geometryData.indexDataSize();

      try {
         const auto vbStagingBuffer =
             allocator.createStagingBuffer(vbSize, "Vertex Staging Buffer");
         void* vbData = allocator.mapMemory(*vbStagingBuffer);
         memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
         allocator.unmapMemory(*vbStagingBuffer);

         // Prepare Index Buffer
         const auto ibStagingBuffer = allocator.createStagingBuffer(ibSize, "Index Staging Buffer");

         const auto data = allocator.mapMemory(*ibStagingBuffer);
         memcpy(data, geometryData.indices.data(), ibSize);
         allocator.unmapMemory(*ibStagingBuffer);

         auto vertexBuffer = allocator.createGpuVertexBuffer(vbSize, "GPU Vertex");
         auto indexBuffer = allocator.createGpuIndexBuffer(ibSize, "GPU Index");
         const auto indicesCount = geometryData.indices.size();

         // Upload Buffers
         context.submit([&](const vk::raii::CommandBuffer& cmd) {
            const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = vbSize};
            cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer->getBuffer(), vbCopy);
            const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = ibSize};
            cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
         });

         auto [imageData, width, height, component] = geometryFactory->getImageData(imageHandle);
         const auto textureHandle = textureList.size();
         textureList.push_back(
             std::make_unique<Textures::Texture>(static_cast<void*>(imageData.data()),
                                                 width,
                                                 height,
                                                 component,
                                                 allocator,
                                                 graphicsDevice.getVulkanDevice(),
                                                 context));

         { // Only need to guard access to the textureInfoList
            ZoneNamedN(c, "Update TextureInfoList", true);
            std::lock_guard lock(textureListMutex);
            LockMark(textureListMutex);
            LockableName(textureListMutex, "Mutate", 6);
            textureInfoList.emplace_back(textureList[textureHandle]->getImageInfo());
         }

         const auto meshHandle = meshList.size();
         meshList.emplace_back(std::move(vertexBuffer), std::move(indexBuffer), indicesCount);

         return cm::ModelData{.meshData = {meshHandle, textureHandle}};

      } catch (const mem::AllocationException& ex) {
         throw ResourceUploadException(
             fmt::format("Error allocating resources for geometry: {0} and image: {1}, {2}",
                         geometryHandle,
                         imageHandle,
                         ex.what()));
      }
   }

   void ResourceManager::accessTextures(
       const std::function<void(const std::vector<vk::DescriptorImageInfo>&)>& fn) const {
      if (textureInfoList.empty()) {
         return;
      }
      std::lock_guard lock(textureListMutex);
      LockableName(textureListMutex, "Access", 6);
      LockMark(textureListMutex);
      fn(textureInfoList);
   }
}