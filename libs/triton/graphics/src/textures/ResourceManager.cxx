#include "textures/ResourceManager.hpp"

#include "cm/Handles.hpp"
#include "cm/LockableResource.hpp"
#include "cm/RenderData.hpp"

#include "as/Model.hpp"

#include "HeightField.hpp"

#include "geometry/GeometryFactory.hpp"
#include "geometry/RenderGroup.hpp"
#include "geometry/GeometryHandles.hpp"

#include "GraphicsDevice.hpp"
#include "VkContext.hpp"

#include "geometry/GeometryData.hpp"

#include "mem/Allocator.hpp"
#include "mem/Buffer.hpp"

namespace tr::gfx::tx {
   using cm::LockableResource;

   ResourceManager::ResourceManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {
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

   auto ResourceManager::getTextures() const
       -> cm::LockableResource<const std::vector<vk::DescriptorImageInfo>> {
      return {textureInfoList, textureListMutex};
   }

   auto ResourceManager::getRenderData() const -> cm::LockableResource<const cm::gpu::RenderData> {
      return {renderData, renderDataMutex};
   }

   auto ResourceManager::createTerrain() -> std::vector<cm::ModelData> {
      ZoneNamedN(zn1, "ResourceManager::createTerrain", true);

      ZoneNamedN(zn2, "Creating Terrain", true);

      // change this to return
      const auto dataHandle = geometryFactory->createTerrain();
      auto modelHandles = std::vector<cm::ModelData>{};

      for (const auto& [geometryHandle, imageHandle] : dataHandle) {
         // TODO(matt) Encode the desired topology in geometryHandle somehow
         const auto modelHandle =
             uploadGeometry(geometryHandle, cm::Topology::Triangles, imageHandle);

         geometryFactory->unload(dataHandle);
         modelHandles.push_back(modelHandle);
      }

      return modelHandles;
   }

   auto ResourceManager::createModel(const std::filesystem::path& filename) noexcept
       -> cm::ModelData {
      ZoneNamedN(zn1, "ResourceManager::loadModel", true);

      ZoneNamedN(zn2, "Loading Model", true);

      // Execute this as an anonymous lambda so we can translate the exception
      auto tritonModelData = [this, &filename]() {
         try {
            return geometryFactory->loadTrm(filename);
         } catch (const BaseException& ex) {
            throw ResourceCreateException("ResourceManager::createModel(): ", ex);
         }
      }();

      // Upload Meshes and Textures and fill out handles in modelData
      auto modelData = [this, &tritonModelData]() {
         try {
            return uploadGeometry(tritonModelData.getGeometryHandle(),
                                  cm::Topology::Triangles,
                                  tritonModelData.getImageHandle());
         } catch (BaseException& ex) {
            ex << "ResourceManager::createModel(): ";
            throw;
         }
      }();

      geometryFactory->unload(
          {{tritonModelData.getGeometryHandle(), tritonModelData.getImageHandle()}});

      if (auto skinData = tritonModelData.getSkinData()) {
         modelData.skinData = std::move(skinData);
      }

      return modelData;
   }

   auto ResourceManager::createAABB(const glm::vec3& min,
                                    const glm::vec3& max) noexcept -> cm::ModelData {
      auto data = geometryFactory->generateAABB(min, max);
      return uploadGeometry(data, cm::Topology::LineList);
   }

   auto ResourceManager::uploadGeometry(const geo::GeometryHandle& geometryHandle,
                                        cm::Topology topology,
                                        std::optional<geo::ImageHandle> imageHandle)
       -> cm::ModelData {
      auto allocator = graphicsDevice.getAllocator();
      auto context = graphicsDevice.getAsyncTransferContext();

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
             allocator->createStagingBuffer(vbSize, "Vertex Staging Buffer");
         void* vbData = allocator->mapMemory(*vbStagingBuffer);
         memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
         allocator->unmapMemory(*vbStagingBuffer);

         // Prepare Index Buffer
         const auto ibStagingBuffer =
             allocator->createStagingBuffer(ibSize, "Index Staging Buffer");

         auto* const data = allocator->mapMemory(*ibStagingBuffer);
         memcpy(data, geometryData.indices.data(), ibSize);
         allocator->unmapMemory(*ibStagingBuffer);

         auto vertexBuffer = allocator->createGpuVertexBuffer(vbSize, "GPU Vertex");
         auto indexBuffer = allocator->createGpuIndexBuffer(ibSize, "GPU Index");
         const auto indicesCount = geometryData.indices.size();

         // Upload Buffers
         context->submit([&](const vk::raii::CommandBuffer& cmd) {
            const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = vbSize};
            cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer->getBuffer(), vbCopy);
            const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = ibSize};
            cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
         });
         size_t textureHandle = 0;
         if (imageHandle) {
            auto [imageData, width, height, component] =
                geometryFactory->getImageData(imageHandle.value());
            textureHandle = textureList.size();
            textureList.push_back(
                std::make_unique<Textures::Texture>(static_cast<void*>(imageData.data()),
                                                    width,
                                                    height,
                                                    component,
                                                    *allocator,
                                                    graphicsDevice.getVulkanDevice(),
                                                    *context));

            { // Only need to guard access to the textureInfoList
               ZoneNamedN(zn1, "Update TextureInfoList", true);
               std::lock_guard lock(textureListMutex);
               LockMark(textureListMutex);
               LockableName(textureListMutex, "Mutate", 6);
               textureInfoList.emplace_back(textureList[textureHandle]->getImageInfo());
            }
         }

         const auto meshHandle = meshList.size();
         meshList.emplace_back(std::move(vertexBuffer), std::move(indexBuffer), indicesCount);

         return cm::ModelData{.meshData = {meshHandle, topology, textureHandle}};

      } catch (const mem::AllocationException& ex) {
         throw ResourceUploadException(
             fmt::format("Error allocating resources for geometry: {0} and image: {1}, {2}",
                         geometryHandle,
                         imageHandle.value(),
                         ex.what()));
      }
   }

} // namespace tr::gfx::tx