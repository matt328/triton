#include "gp/DefaultGameplaySystem.hpp"
#include "commands/CreateStaticEntity.hpp"
#include "commands/CreateTestEntity.hpp"
#include "gp/action/IActionSystem.hpp"
#include "gp/components/Resources.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "tr/IEventBus.hpp"
#include "commands/CreateCamera.hpp"
#include "commands/CreateAnimatedEntity.hpp"

namespace tr {

constexpr auto DefaultFOV = 60.f;
constexpr auto DefaultNearClip = 0.1f;
constexpr auto DefaultFarClip = 10000.f;
constexpr auto DefaultPosition = glm::vec3{0.f, 0.f, 5.f};

DefaultGameplaySystem::DefaultGameplaySystem(std::shared_ptr<IEventBus> newEventBus,
                                             std::shared_ptr<AssetManager> newAssetManager,
                                             std::shared_ptr<IActionSystem> newActionSystem)
    : eventBus{std::move(newEventBus)},
      assetManager{std::move(newAssetManager)},
      actionSystem{std::move(newActionSystem)} {

  registry = std::make_shared<entt::registry>();

  cameraSystem = std::make_shared<CameraSystem>(eventBus, *registry);
  transformSystem = std::make_shared<TransformSystem>();
  renderDataSystem = std::make_shared<RenderDataSystem>();
  animationSystem = std::make_shared<AnimationSystem>(assetManager);

  entityCreatedConnection =
      registry->on_construct<entt::entity>().connect<&DefaultGameplaySystem::entityCreated>(this);

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
  registry->ctx().insert_or_assign<WindowDimensions>(
      WindowDimensions{.width = event.width, .height = event.height});
}

auto DefaultGameplaySystem::handleSwapchainCreated(const SwapchainCreated& event) -> void {
  registry->ctx().insert_or_assign<WindowDimensions>(
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

    renderDataSystem->update(*registry, renderData);
  }
  transferHandler(renderData);
}

void DefaultGameplaySystem::fixedUpdate() {
  ZoneNamedN(var, "FixedUpdate", true);
  {
    ZoneNamedN(camZone, "CameraSystem", true);
    cameraSystem->fixedUpdate(*registry);
  }
  {
    ZoneNamedN(xformZone, "Transform", true);
    transformSystem->update(*registry);
  }
  {
    ZoneNamedN(var, "Animation", true);
    animationSystem->update(*registry);
  }
}

void DefaultGameplaySystem::setRenderDataTransferHandler(const RenderDataTransferHandler& handler) {
  this->transferHandler = handler;
}

auto DefaultGameplaySystem::createStaticModelEntity(std::string filename,
                                                    std::string_view entityName,
                                                    std::optional<Transform> initialTransform)
    -> void {
  auto modelData = assetManager->loadModel(filename);

  auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                             .position = glm::zero<glm::vec3>(),
                             .transformation = glm::identity<glm::mat4>()};

  if (initialTransform.has_value()) {
    transform.position = initialTransform->position;
    transform.rotation = initialTransform->rotation;
  }

  {
    std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
    LockMark(registryMutex);
    const auto entity = registry->create();
    registry->emplace<Renderable>(entity, std::vector{modelData.meshData});
    registry->emplace<Transform>(entity, transform);
    registry->emplace<EditorInfo>(entity, entityName.data());
  }
}

auto DefaultGameplaySystem::createAnimatedModelEntity(const AnimatedModelData& modelData,
                                                      std::optional<Transform> initialTransform)
    -> void {

  auto loadedModelData = assetManager->loadModel(modelData.modelFilename);

  assert(loadedModelData.skinData.has_value());

  loadedModelData.animationData = std::make_optional(
      AnimationData{.skeletonHandle = assetManager->loadSkeleton(modelData.skeletonFilename),
                    .animationHandle = assetManager->loadAnimation(modelData.animationFilename)});

  auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                             .position = glm::zero<glm::vec3>(),
                             .transformation = glm::identity<glm::mat4>()};
  {
    std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
    LockMark(registryMutex);

    if (initialTransform.has_value()) {
      transform.position = initialTransform->position;
      transform.rotation = initialTransform->rotation;
    }

    const auto entity = registry->create();
    registry->emplace<Animation>(entity,
                                 loadedModelData.animationData->animationHandle,
                                 loadedModelData.animationData->skeletonHandle,
                                 loadedModelData.skinData->jointMap,
                                 loadedModelData.skinData->inverseBindMatrices);
    registry->emplace<Transform>(entity, transform);
    registry->emplace<Renderable>(entity, std::vector{loadedModelData.meshData});
    registry->emplace<EditorInfo>(entity, modelData.entityName.value_or("Unnamed Entity"));
  }
}

auto DefaultGameplaySystem::createTerrain() -> void {
  Log.trace("Creating terrain");
}

auto DefaultGameplaySystem::createDefaultCamera() -> void {
  const auto [width, height] = registry->ctx().get<const WindowDimensions>();

  auto cameraInfo = CameraInfo{
      .width = width,
      .height = height,
      .fov = DefaultFOV,
      .nearClip = DefaultNearClip,
      .farClip = DefaultFarClip,
      .position = DefaultPosition,
  };

  {
    std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
    LockMark(registryMutex);

    const auto entity = registry->create();
    registry->emplace<Camera>(entity,
                              cameraInfo.width,
                              cameraInfo.height,
                              cameraInfo.fov,
                              cameraInfo.nearClip,
                              cameraInfo.farClip,
                              cameraInfo.position);
    registry->emplace<EditorInfo>(entity, "Default Camera");
    registry->ctx().insert_or_assign<CurrentCamera>(CurrentCamera{entity});
  }
}

auto DefaultGameplaySystem::createTestEntity([[maybe_unused]] std::string_view name) -> void {
  Log.trace("Creating test entity: {}", name.data());
  auto modelData = assetManager->createCube();

  {
    std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
    LockMark(registryMutex);

    const auto entity = registry->create();
    registry->emplace<Renderable>(entity, std::vector{modelData.meshData});
    registry->emplace<EditorInfo>(entity, name.data());
    registry->emplace<Transform>(entity);
  }
}

auto DefaultGameplaySystem::removeEntity(tr::EntityType entity) -> void {
  std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);
  registry->destroy(entity);
}

auto DefaultGameplaySystem::getRegistry() const -> std::shared_ptr<entt::registry> {
  return registry;
}

auto DefaultGameplaySystem::entityCreated([[maybe_unused]] entt::registry& reg,
                                          [[maybe_unused]] entt::entity entity) const -> void {
  Log.trace("Entity Created: {}", static_cast<uint32_t>(entity));
  eventBus->emit(EntityCreated{entity});
}

}
