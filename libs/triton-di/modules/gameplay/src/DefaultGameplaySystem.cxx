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

  commandQueue =
      std::make_unique<CommandQueue<entt::registry&, const std::shared_ptr<AssetManager>&>>();

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
    renderData.staticMeshData.clear();
    renderData.terrainMeshData.clear();
    renderData.skinnedMeshData.clear();
    renderData.animationData.clear();
    renderData.staticGpuMeshData.clear();

    {
      std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
      LockMark(registryMutex);
      renderDataSystem->update(*registry, renderData);
    }
  }
  transferHandler(renderData);
}

void DefaultGameplaySystem::fixedUpdate() {
  std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  {
    ZoneNamedN(z, "Gameplay Command Queue", true);
    commandQueue->processCommands(*registry, assetManager);
  }
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
  commandQueue->enqueue(
      std::make_unique<CreateStaticEntityCommand>(filename, entityName.data(), initialTransform));
}

auto DefaultGameplaySystem::createAnimatedModelEntity(
    [[maybe_unused]] const AnimatedModelData& modelData) -> void {
  commandQueue->enqueue(std::make_unique<CreateAnimatedEntity>(modelData));
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

  commandQueue->enqueue(std::make_unique<CreateCamera>(cameraInfo));
}

auto DefaultGameplaySystem::createTestEntity([[maybe_unused]] std::string_view name) -> void {
  Log.trace("Creating test entity: {}", name.data());
  commandQueue->enqueue(std::make_unique<CreateTestEntityCommand>(name));
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
