#include "DefaultGameplaySystem.hpp"

#include "EntityService.hpp"
#include "fx/IActionSystem.hpp"
#include "as/Model.hpp"
#include "fx/IEventBus.hpp"
#include "fx/IResourceProxy.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "fx/ext/ITerrainSystemProxy.hpp"

namespace tr {

constexpr auto DefaultFOV = 60.f;
constexpr auto DefaultNearClip = 0.1f;
constexpr auto DefaultFarClip = 10000.f;
constexpr auto DefaultPosition = glm::vec3{0.f, 7.f, 5.f};

DefaultGameplaySystem::DefaultGameplaySystem(
    std::shared_ptr<IEventBus> newEventBus,
    std::shared_ptr<IAssetService> newAssetService,
    std::shared_ptr<IActionSystem> newActionSystem,
    std::shared_ptr<CameraSystem> newCameraSystem,
    std::shared_ptr<TransformSystem> newTransformSystem,
    std::shared_ptr<AnimationSystem> newAnimationSystem,
    std::shared_ptr<RenderDataSystem> newRenderDataSystem,
    std::shared_ptr<EntityService> newEntityService,
    std::shared_ptr<ITerrainSystemProxy> newTerrainSystemProxy,
    std::shared_ptr<IResourceProxy> newResourceProxy)
    : eventBus{std::move(newEventBus)},
      assetService{std::move(newAssetService)},
      actionSystem{std::move(newActionSystem)},
      cameraSystem{std::move(newCameraSystem)},
      transformSystem{std::move(newTransformSystem)},
      animationSystem{std::move(newAnimationSystem)},
      renderDataSystem{std::move(newRenderDataSystem)},
      entityService{std::move(newEntityService)},
      terrainSystemProxy{std::move(newTerrainSystemProxy)},
      resourceProxy{std::move(newResourceProxy)} {

  entityCreatedConnection = entityService->registerEntityCreated(
      [this](entt::registry& reg, entt::entity entity) { entityCreated(reg, entity); });

  eventBus->subscribe<SwapchainResized>(
      [&](const SwapchainResized& event) { handleSwapchainResized(event); });

  eventBus->subscribe<SwapchainCreated>(
      [&](const SwapchainCreated& event) { handleSwapchainCreated(event); });

  // Forward
  actionSystem->mapSource(Source{Key::Up, SourceType::Boolean},
                          StateType::State,
                          ActionType::MoveForward);
  actionSystem->mapSource(Source{Key::W, SourceType::Boolean},
                          StateType::State,
                          ActionType::MoveForward);

  // Backward
  actionSystem->mapSource(Source{Key::Down, SourceType::Boolean},
                          StateType::State,
                          ActionType::MoveBackward);
  actionSystem->mapSource(Source{Key::S, SourceType::Boolean},
                          StateType::State,
                          ActionType::MoveBackward);
  // Left
  actionSystem->mapSource(Source{Key::Left, SourceType::Boolean},
                          StateType::State,
                          ActionType::StrafeLeft);
  actionSystem->mapSource(Source{Key::A, SourceType::Boolean},
                          StateType::State,
                          ActionType::StrafeLeft);
  // Right
  actionSystem->mapSource(Source{Key::Right, SourceType::Boolean},
                          StateType::State,
                          ActionType::StrafeRight);
  actionSystem->mapSource(Source{Key::D, SourceType::Boolean},
                          StateType::State,
                          ActionType::StrafeRight);
  // Look
  actionSystem->mapSource(Source{MouseInput::MOVE_X, SourceType::Float},
                          StateType::Range,
                          ActionType::LookHorizontal);
  actionSystem->mapSource(Source{MouseInput::MOVE_Y, SourceType::Float},
                          StateType::Range,
                          ActionType::LookVertical);
}

DefaultGameplaySystem::~DefaultGameplaySystem() {
  Log.trace("Destroying Gameplay System");
  entityCreatedConnection.release();
}

auto DefaultGameplaySystem::handleSwapchainResized(const SwapchainResized& event) -> void {
  entityService->updateWindowDimensions(
      WindowDimensions{.width = event.width, .height = event.height});
}

auto DefaultGameplaySystem::handleSwapchainCreated(const SwapchainCreated& event) -> void {
  entityService->updateWindowDimensions(
      WindowDimensions{.width = event.width, .height = event.height});
}

void DefaultGameplaySystem::update() {
  {
    ZoneNamedN(rd, "RenderData", true);
    renderData.objectData.clear();
    renderData.dynamicObjectData.clear();
    renderData.staticMeshData.clear();
    renderData.terrainMeshData.clear();
    renderData.dynamicMeshData.clear();
    renderData.animationData.clear();
    renderData.staticGpuMeshData.clear();

    renderDataSystem->update(renderData);
  }
  assert(transferHandler && "TransferHandler was not set");
  transferHandler(renderData);
}

void DefaultGameplaySystem::fixedUpdate() {
  ZoneNamedN(var, "FixedUpdate", true);
  {
    ZoneNamedN(camZone, "CameraSystem", true);
    cameraSystem->fixedUpdate();
  }
  {
    ZoneNamedN(xformZone, "Transform", true);
    transformSystem->update();
  }
  {
    ZoneNamedN(var, "Animation", true);
    animationSystem->update();
  }
}

void DefaultGameplaySystem::setRenderDataTransferHandler(const RenderDataTransferHandler& handler) {
  this->transferHandler = handler;
}

auto DefaultGameplaySystem::createStaticModelEntity(std::string filename,
                                                    std::string_view entityName,
                                                    std::optional<TransformData> initialTransform)
    -> GameObjectId {
  const auto model = assetService->loadModel(filename);

  auto modelData = resourceProxy->uploadModel(model);

  auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                             .position = glm::zero<glm::vec3>(),
                             .transformation = glm::identity<glm::mat4>()};

  if (initialTransform.has_value()) {
    transform.position = initialTransform->position;
    transform.rotation = initialTransform->rotation;
  }

  return static_cast<GameObjectId>(
      entityService->createStaticEntity(std::vector{modelData.meshData}, transform, entityName));
}

auto DefaultGameplaySystem::createAnimatedModelEntity(const AnimatedModelData& modelData,
                                                      std::optional<TransformData> initialTransform)
    -> GameObjectId {

  const auto model = assetService->loadModel(modelData.modelFilename);

  auto loadedModelData = resourceProxy->uploadModel(model);

  assert(loadedModelData.skinData.has_value());

  loadedModelData.animationData = std::make_optional(
      AnimationData{.skeletonHandle = assetService->loadSkeleton(modelData.skeletonFilename),
                    .animationHandle = assetService->loadAnimation(modelData.animationFilename)});

  auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                             .position = glm::zero<glm::vec3>(),
                             .transformation = glm::identity<glm::mat4>()};

  if (initialTransform.has_value()) {
    transform.position = initialTransform->position;
    transform.rotation = initialTransform->rotation;
  }

  return static_cast<GameObjectId>(
      entityService->createDynamicEntity(loadedModelData,
                                         transform,
                                         modelData.entityName.value_or("Unnamed Entity")));
}

auto DefaultGameplaySystem::createTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& {
  auto& terrainResult = terrainSystemProxy->registerTerrain(createInfo);
  entityService->createTerrain(terrainResult);
  return terrainResult;
}

auto DefaultGameplaySystem::createDefaultCamera() -> void {
  auto cameraInfo = CameraInfo{
      .fov = DefaultFOV,
      .nearClip = DefaultNearClip,
      .farClip = DefaultFarClip,
      .position = DefaultPosition,
  };
  entityService->createCamera(cameraInfo, "Default Camera");
}

auto DefaultGameplaySystem::removeEntity(GameObjectId entity) -> void {
  entityService->removeEntity(static_cast<EntityType>(entity));
}

auto DefaultGameplaySystem::entityCreated([[maybe_unused]] entt::registry& reg,
                                          entt::entity entity) const -> void {
  eventBus->emit(EntityCreated{static_cast<GameObjectId>(entity)});
}

// This method is just debug. Eventually push this down into the TerrainSystem
auto DefaultGameplaySystem::triangulateChunk(GameObjectId terrainId,
                                             GameObjectId chunkId,
                                             glm::ivec3 cellPosition) -> void {
  const auto terrainHandle = entityService->getTerrainHandle(static_cast<EntityId>(terrainId));
  const auto chunkHandle = entityService->getChunkHandle(static_cast<EntityId>(chunkId));
  terrainSystemProxy->triangulateBlock(terrainHandle, chunkHandle, chunkId, cellPosition);
}

}
