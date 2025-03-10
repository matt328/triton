#include "gp/DefaultGameplaySystem.hpp"
#include "gp/action/IActionSystem.hpp"
#include "gp/components/Resources.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "tr/IEventBus.hpp"
#include "commands/CreateCamera.hpp"
#include "tr/TerrainManager.hpp"

namespace tr {

constexpr auto DefaultFOV = 60.f;
constexpr auto DefaultNearClip = 0.1f;
constexpr auto DefaultFarClip = 10000.f;
constexpr auto DefaultPosition = glm::vec3{0.f, 0.f, 5.f};

DefaultGameplaySystem::DefaultGameplaySystem(std::shared_ptr<IEventBus> newEventBus,
                                             std::shared_ptr<AssetManager> newAssetManager,
                                             std::shared_ptr<IActionSystem> newActionSystem,
                                             std::shared_ptr<CameraSystem> newCameraSystem,
                                             std::shared_ptr<TransformSystem> newTransformSystem,
                                             std::shared_ptr<AnimationSystem> newAnimationSystem,
                                             std::shared_ptr<RenderDataSystem> newRenderDataSystem,
                                             std::shared_ptr<EntityService> newEntityService,
                                             std::shared_ptr<ITerrainSystem> newTerrainSystem)
    : eventBus{std::move(newEventBus)},
      assetManager{std::move(newAssetManager)},
      actionSystem{std::move(newActionSystem)},
      cameraSystem{std::move(newCameraSystem)},
      transformSystem{std::move(newTransformSystem)},
      animationSystem{std::move(newAnimationSystem)},
      renderDataSystem{std::move(newRenderDataSystem)},
      entityService{std::move(newEntityService)},
      terrainSystem{std::move(newTerrainSystem)} {

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
                                                    std::optional<Transform> initialTransform)
    -> tr::EntityType {
  auto modelData = assetManager->loadModel(filename);

  auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                             .position = glm::zero<glm::vec3>(),
                             .transformation = glm::identity<glm::mat4>()};

  if (initialTransform.has_value()) {
    transform.position = initialTransform->position;
    transform.rotation = initialTransform->rotation;
  }

  return entityService->createStaticEntity(std::vector{modelData.meshData}, transform, entityName);
}

auto DefaultGameplaySystem::createAnimatedModelEntity(const AnimatedModelData& modelData,
                                                      std::optional<Transform> initialTransform)
    -> tr::EntityType {

  auto loadedModelData = assetManager->loadModel(modelData.modelFilename);

  assert(loadedModelData.skinData.has_value());

  loadedModelData.animationData = std::make_optional(
      AnimationData{.skeletonHandle = assetManager->loadSkeleton(modelData.skeletonFilename),
                    .animationHandle = assetManager->loadAnimation(modelData.animationFilename)});

  auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                             .position = glm::zero<glm::vec3>(),
                             .transformation = glm::identity<glm::mat4>()};

  if (initialTransform.has_value()) {
    transform.position = initialTransform->position;
    transform.rotation = initialTransform->rotation;
  }

  return entityService->createDynamicEntity(loadedModelData,
                                            transform,
                                            modelData.entityName.value_or("Unnamed Entity"));
}

auto DefaultGameplaySystem::createTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& {
  auto& terrainResult = terrainSystem->registerTerrain(createInfo);
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

auto DefaultGameplaySystem::createTestEntity([[maybe_unused]] std::string_view name) -> void {
  Log.trace("Creating test entity: {}", name.data());
  auto modelData = assetManager->createCube();
  entityService->createStaticEntity(std::vector{modelData.meshData}, Transform{}, name);
}

auto DefaultGameplaySystem::removeEntity(tr::EntityType entity) -> void {
  entityService->removeEntity(entity);
}

auto DefaultGameplaySystem::entityCreated([[maybe_unused]] entt::registry& reg,
                                          entt::entity entity) const -> void {
  eventBus->emit(EntityCreated{entity});
}

auto DefaultGameplaySystem::getEntityService() const -> std::shared_ptr<EntityService> {
  return entityService;
}

// This method is just debug. Eventually push this down into the TerrainSystem
auto DefaultGameplaySystem::triangulateChunk(tr::EntityType terrainId,
                                             tr::EntityType chunkId,
                                             glm::ivec3 cellPosition) -> void {
  const auto terrainHandle = entityService->getTerrainHandle(terrainId);
  const auto chunkHandle = entityService->getChunkHandle(chunkId);
  terrainSystem->triangulateBlock(terrainHandle, chunkHandle, chunkId, cellPosition);
}

}
