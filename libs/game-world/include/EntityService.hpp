#pragma once

#include "EntitySystemTypes.hpp"

#include "api/gw/RenderData.hpp"
#include "components/TerrainChunk.hpp"
#include "components/Animation.hpp"
#include "components/Camera.hpp"
#include "components/EditorInfo.hpp"
#include "components/Renderable.hpp"
#include "components/Resources.hpp"
#include "components/Transform.hpp"

namespace tr {

class IAssetService;

using CamFn = std::function<void(RenderData& renderData, const Camera&)>;

using StaticUpdateFn =
    std::function<void(RenderData& renderData, entt::entity, const Renderable&, const Transform&)>;

using DynamicUpdateFn = std::function<void(RenderData& renderData,
                                           entt::entity,
                                           const Animation&,
                                           const Renderable&,
                                           const Transform&)>;

using TerrainUpdateFn =
    std::function<void(RenderData&, entt::entity, const ChunkComponent&, const Renderable&)>;

using EntityCreatedFn = std::function<void(entt::registry&, entt::entity)>;

using EntityId = entt::entity;

class EntityService {
public:
  explicit EntityService(std::shared_ptr<IAssetService> newAssetService);
  ~EntityService();

  EntityService(const EntityService&) = delete;
  EntityService(EntityService&&) = delete;
  auto operator=(const EntityService&) -> EntityService& = delete;
  auto operator=(EntityService&&) -> EntityService& = delete;

  auto updateAnimations(const std::function<void(entt::entity, Animation&)>& fn) -> void;

  auto updateCameraActions(const std::function<void(entt::entity, Camera&)>& fn) -> void;

  auto updateCameras(
      const std::function<void(uint32_t width, uint32_t height, entt::entity, Camera&)>& fn)
      -> void;

  auto updateTransforms(const std::function<void(entt::entity, Transform&)>& fn) -> void;

  auto updateRenderData(const CamFn& camUpdateFn,
                        const StaticUpdateFn& staticFn,
                        const DynamicUpdateFn& dynamicFn,
                        const TerrainUpdateFn& terrainFn,
                        RenderData& renderData) -> void;

  auto registerEntityCreated(const EntityCreatedFn& fn) -> entt::connection;

  auto updateWindowDimensions(WindowDimensions windowDimensions) -> void;

  auto createStaticEntity(std::vector<MeshData> meshData,
                          Transform transform,
                          std::string_view name) -> EntityId;

  auto createDynamicEntity(ModelData modelData,
                           Transform transform,
                           std::string_view name) -> EntityId;

  auto createTerrain(TerrainResult2& terrainResult) -> void;

  auto createCamera(CameraInfo cameraInfo, std::string_view name, bool setDefault = true) -> void;

  auto removeEntity(EntityType entity) -> void;

  auto forEachEditorInfo(const std::function<void(EntityId entity, EditorInfo& editorInfo)>& fn)
      -> void;

  auto setTransform(EntityId entityId, Transform transform) -> void;

  auto getTerrainHandle(EntityId terrainId) -> TerrainHandle;
  auto getChunkHandle(EntityId chunkId) -> ChunkHandle;

  auto addMeshToTerrainChunk(EntityId terrainId, EntityId chunkId, MeshHandle meshHandle) -> void;

private:
  std::shared_ptr<IAssetService> assetService;

  std::unique_ptr<entt::registry> registry;
  mutable TracySharedLockableN(std::shared_mutex, registryMutex, "EntityService");

  EntityCreatedFn entityCreatedFn;

  auto entityCreated([[maybe_unused]] entt::registry& reg,
                     [[maybe_unused]] entt::entity entity) const -> void;
};

}
