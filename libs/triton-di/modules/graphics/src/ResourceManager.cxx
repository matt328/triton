#include "ResourceManager.hpp"

#include "ResourceExceptions.hpp"
#include "cm/Handles.hpp"
#include "cm/LockableResource.hpp"

#include "geo/GeometryData.hpp"
#include "geo/GeometryFactory.hpp"
#include "geo/GeometryHandles.hpp"

#include "gfx/IGraphicsDevice.hpp"

#include "as/Model.hpp"

namespace tr::gfx {

   ResourceManager::ResourceManager(std::shared_ptr<IGraphicsDevice> newGraphicsDevice)
       : graphicsDevice{std::move(newGraphicsDevice)} {
      geometryFactory = std::make_unique<geo::GeometryFactory>();
   }

   auto ResourceManager::createTerrain() -> std::vector<cm::ModelData> {
      ZoneNamedN(zn2, "Creating Terrain", true);

      const auto dataHandle = geometryFactory->createTerrain();
      auto modelDataList = std::vector<cm::ModelData>{};

      for (const auto& [geometryHandle, imageHandle] : dataHandle) {
         const auto meshHandle =
             graphicsDevice->uploadVertexData(geometryFactory->getGeometryData(geometryHandle));
         const auto textureHandle =
             graphicsDevice->uploadImageData(geometryFactory->getImageData(imageHandle));

         modelDataList.push_back(
             cm::ModelData{.meshData = cm::MeshData{.meshHandle = meshHandle,
                                                    .topology = geometryHandle.topology,
                                                    .textureHandle = textureHandle}});
      }
      geometryFactory->unload(dataHandle);
      return modelDataList;
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
            const auto geometryData =
                geometryFactory->getGeometryData(tritonModelData.getGeometryHandle());
            const auto imageData = geometryFactory->getImageData(tritonModelData.getImageHandle());

            const auto meshHandle = graphicsDevice->uploadVertexData(geometryData);
            const auto textureHandle = graphicsDevice->uploadImageData(imageData);

            return cm::ModelData{.meshData = cm::MeshData{
                                     .meshHandle = meshHandle,
                                     .topology = cm::Topology::Triangles,
                                     .textureHandle = textureHandle,
                                 }};

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
      auto geometryHandle = geometryFactory->generateAABB(min, max);
      auto meshHandle =
          graphicsDevice->uploadVertexData(geometryFactory->getGeometryData(geometryHandle));
      return cm::ModelData{
          .meshData = cm::MeshData{.meshHandle = meshHandle, .topology = cm::Topology::LineList}};
   }

}
