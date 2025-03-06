#include "gp/EntityService.hpp"
#include "gp/components/TerrainComponent.hpp"
#include "tr/TerrainManager.hpp"
#include "gp/components/EditorInfo.hpp"
#include "gp/components/Resources.hpp"
#include "gp/components/TerrainChunk.hpp"

namespace tr {

EntityService::EntityService(std::shared_ptr<AssetManager> newAssetManager)
    : assetManager{std::move(newAssetManager)} {
  registry = std::make_unique<entt::registry>();
}

EntityService::~EntityService() {
}

auto EntityService::registerEntityCreated(const EntityCreatedFn& fn) -> entt::connection {
  entityCreatedFn = fn;
  return registry->on_construct<entt::entity>().connect<&EntityService::entityCreated>(this);
}

auto EntityService::updateAnimations(const std::function<void(entt::entity, Animation&)>& fn)
    -> void {
  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  for (const auto view = registry->view<Animation>(); auto [entity, animationData] : view.each()) {
    fn(entity, animationData);
  }
}

auto EntityService::updateCameraActions(const std::function<void(entt::entity, Camera&)>& fn)
    -> void {

  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  for (const auto view = registry->view<Camera>(); auto [entity, cameraData] : view.each()) {
    fn(entity, cameraData);
  }
}

auto EntityService::updateCameras(
    const std::function<void(uint32_t width, uint32_t height, entt::entity, Camera&)>& fn) -> void {

  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  const auto view = registry->view<Camera>();
  const auto [width, height] = registry->ctx().get<const WindowDimensions>();

  for (auto [entity, cam] : view.each()) {
    fn(width, height, entity, cam);
  }
}

auto EntityService::updateTransforms(const std::function<void(entt::entity, Transform&)>& fn)
    -> void {

  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  for (const auto view = registry->view<Transform>(); auto [entity, transform] : view.each()) {
    fn(entity, transform);
  }
}

/// Pulls data from the ECS Components and converts it into the RenderData struct
auto EntityService::updateRenderData(const CamFn& camUpdateFn,
                                     const StaticUpdateFn& staticFn,
                                     const DynamicUpdateFn& dynamicFn,
                                     const TerrainUpdateFn& terrainFn,
                                     RenderData& renderData) -> void {

  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  if (!registry->ctx().contains<const CurrentCamera>()) {
    Log.trace("Context doesn't contain current camera");
    return;
  }

  const auto cameraEntity = registry->ctx().get<const CurrentCamera>();
  const auto cam = registry->get<Camera>(cameraEntity.currentCamera);

  camUpdateFn(renderData, cam);

  // Process Static Objects
  for (const auto view = registry->view<Renderable, Transform>(entt::exclude<Animation>);
       const auto& [entity, renderable, transform] : view.each()) {
    staticFn(renderData, entity, renderable, transform);
  }

  // Process Dynamic Objects
  const auto animationsView = registry->view<Animation, Renderable, Transform>();
  for (const auto& [entity, animationData, renderable, transform] : animationsView.each()) {
    dynamicFn(renderData, entity, animationData, renderable, transform);
  }

  // Process Terrain Entities
  const auto terrainView = registry->view<TerrainComponent>();
  for (const auto& [entity, terrain] : terrainView.each()) {
    terrainFn(renderData, entity, terrain);
  }
}

auto EntityService::entityCreated([[maybe_unused]] entt::registry& reg,
                                  [[maybe_unused]] entt::entity entity) const -> void {
  entityCreatedFn(reg, entity);
}

auto EntityService::updateWindowDimensions(WindowDimensions windowDimensions) -> void {
  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);
  registry->ctx().insert_or_assign<WindowDimensions>(std::move(windowDimensions));
}

auto EntityService::createStaticEntity(std::vector<MeshData> meshData,
                                       Transform transform,
                                       std::string_view name) -> tr::EntityType {
  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);
  const auto entity = registry->create();
  registry->emplace<Renderable>(entity, meshData);
  registry->emplace<Transform>(entity, transform);
  registry->emplace<EditorInfo>(entity, name.data());
  return entity;
}

auto EntityService::createDynamicEntity(ModelData modelData,
                                        Transform transform,
                                        std::string_view name) -> tr::EntityType {

  const auto& skeleton = assetManager->getSkeleton(modelData.animationData->skeletonHandle);

  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  const auto entity = registry->create();
  registry->emplace<Animation>(entity,
                               modelData.animationData->animationHandle,
                               modelData.animationData->skeletonHandle,
                               modelData.skinData->jointMap,
                               modelData.skinData->inverseBindMatrices,
                               skeleton.num_joints(),
                               skeleton.num_soa_joints());
  registry->emplace<Transform>(entity, transform);
  registry->emplace<Renderable>(entity, std::vector{modelData.meshData});
  registry->emplace<EditorInfo>(entity, name.data());

  return entity;
}

auto EntityService::getTerrainHandle(tr::EntityType terrainId) -> TerrainHandle {
  const auto terrainComponent = registry->get<TerrainComponent>(terrainId);
  return terrainComponent.handle;
}

auto EntityService::getChunkHandle(tr::EntityType chunkId) -> ChunkHandle {
  const auto chunkComponent = registry->get<ChunkComponent>(chunkId);
  return chunkComponent.handle;
}

/* TODO(matt): Tomorrow, un-f all of this confusion.

  TerrainManager has its internal structs it uses.
  When creating a terrain, you ask the terrain manager to create one with the given params. It
  creates it and adds a number of chunks. It should put all chunks into a single map, not inside the
  TerrainDefinition. Only put a vector of chunkhandles inside the TerrainDefinition.

  TerrainManager

  UI -> GameplaySystem -> TerrainManager(Create) -> GameplaySystem -> EntityService -> UI

  The issue is the UI needs a TerrainName,EntityId,TerrainHandle, and a list of
  ChunkName,ChunkEntityId,ChunkHandle in order to poke at the terrain and the chunk.

  Also for each Chunk, the UI needs to know how many cells, which it can determine, chunkSize - 2.
*/

/*
  DataStore can organize and index these by whatever makes most sense, probably name->TerrainData,
  and name->ChunkData
*/

auto EntityService::createTerrain(TerrainResult2& terrainResult) -> void {
  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  auto chunkEntityIds = std::vector<tr::EntityType>{};

  // Create an entity for each chunk
  for (auto& chunk : terrainResult.chunks) {
    const auto entity = registry->create();
    chunk.entityId = entity;
    chunkEntityIds.push_back(entity);
    const auto chunkName =
        fmt::format("Chunk({}, {}, {})", chunk.location.x, chunk.location.y, chunk.location.z);
    registry->emplace<EditorInfo>(entity, chunkName);
    registry->emplace<ChunkComponent>(entity, chunk.location, chunk.size, chunk.chunkHandle);
  }

  const auto entity = registry->create();
  terrainResult.entityId = entity;
  registry->emplace<TerrainComponent>(entity,
                                      terrainResult.name,
                                      terrainResult.terrainHandle,
                                      chunkEntityIds);
  registry->emplace<EditorInfo>(entity, terrainResult.name);
}

auto EntityService::createCamera(CameraInfo cameraInfo,
                                 std::string_view name,
                                 bool setDefault) -> void {

  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  const auto [width, height] = registry->ctx().get<const WindowDimensions>();

  const auto entity = registry->create();
  registry->emplace<Camera>(entity,
                            width,
                            height,
                            cameraInfo.fov,
                            cameraInfo.nearClip,
                            cameraInfo.farClip,
                            cameraInfo.position);
  registry->emplace<EditorInfo>(entity, name.data());
  if (setDefault) {
    registry->ctx().insert_or_assign<CurrentCamera>(CurrentCamera{entity});
  }
}

auto EntityService::forEachEditorInfo(
    const std::function<void(tr::EntityType entity, EditorInfo& editorInfo)>& fn) -> void {

  std::shared_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  auto view = registry->view<tr::EditorInfo>();
  for (auto [entity, editorInfo] : view.each()) {
    fn(entity, editorInfo);
  }
}

auto EntityService::removeEntity(EntityType entity) -> void {
  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);
  registry->destroy(entity);
}

auto EntityService::setTransform(tr::EntityType entityId, Transform transform) -> void {
  std::unique_lock<SharedLockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);
  registry->patch<Transform>(entityId, [transform](Transform& t) {
    t.position = transform.position;
    t.rotation = transform.rotation;
  });
}

}
