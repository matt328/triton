#include "EntityManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/fx/IStateBuffer.hpp"
#include "components/Renderable.hpp"
#include "components/Resources.hpp"
#include "components/Transform.hpp"
#include "components/EditorInfo.hpp"
#include "systems2/CameraHandler.hpp"
#include "systems2/FinalizerSystem.hpp"

namespace tr {

constexpr auto DefaultFOV = 60.f;
constexpr auto DefaultNearClip = 0.1f;
constexpr auto DefaultFarClip = 10000.f;
constexpr auto DefaultPosition = glm::vec3{0.f, 0.f, 5.f};

EntityManager::EntityManager(std::shared_ptr<IEventQueue> newEventQueue,
                             std::shared_ptr<IStateBuffer> newStateBuffer)
    : eventQueue{std::move(newEventQueue)}, stateBuffer{std::move(newStateBuffer)} {
  registry = std::make_unique<entt::registry>();
  Log.trace("Created EntityManager");

  eventQueue->subscribe<Action>(
      [this](const Action& event) { CameraHandler::handleAction(event, *registry); });

  eventQueue->subscribe<SwapchainCreated>(
      [this](const SwapchainCreated& event) { renderAreaCreated(event); });

  eventQueue->subscribe<SwapchainResized>(
      [this](const SwapchainResized& event) { renderAreaResized(event); });

  eventQueue->subscribe<StaticModelUploaded>(
      [this](const StaticModelUploaded& event) { registerStaticModel(event); });
}

EntityManager::~EntityManager() {
  Log.trace("Destroying EntityManager");
}

auto EntityManager::update() -> void {
  Timestamp currentTime = std::chrono::steady_clock::now();
  SimState writeState = SimState{1};
  FinalizerSystem::update(*registry, writeState, currentTime);
  stateBuffer->pushState(writeState, currentTime);
}

auto EntityManager::renderAreaCreated(const SwapchainCreated& event) -> void {
  registry->ctx().insert_or_assign<WindowDimensions>(
      WindowDimensions{.width = event.width, .height = event.height});
  createDefaultCamera();
}

auto EntityManager::renderAreaResized(const SwapchainResized& event) -> void {
  registry->ctx().insert_or_assign<WindowDimensions>(
      WindowDimensions{.width = event.width, .height = event.height});
}

auto EntityManager::createDefaultCamera() -> void {
  auto cameraInfo = CameraInfo{
      .fov = DefaultFOV,
      .nearClip = DefaultNearClip,
      .farClip = DefaultFarClip,
      .position = DefaultPosition,
  };
  const auto [width, height] = registry->ctx().get<const WindowDimensions>();

  const auto entity = registry->create();
  registry->emplace<Camera>(entity,
                            width,
                            height,
                            cameraInfo.fov,
                            cameraInfo.nearClip,
                            cameraInfo.farClip,
                            cameraInfo.position);
  registry->emplace<EditorInfo>(entity, "Default Camera");
  registry->ctx().insert_or_assign<CurrentCamera>(CurrentCamera{entity});
}

auto EntityManager::registerStaticModel(const StaticModelUploaded& event) -> void {
  ZoneScopedN("static model uploaded handler");
  Log.trace("EntityManager handling StaticModelUploaded, geometryHandle={}",
            event.geometryHandle.id);
  auto entityId = registry->create();

  registry->emplace<Renderable>(entityId, std::vector<Handle<Geometry>>{event.geometryHandle});
  registry->emplace<Transform>(entityId);

  eventQueue->emit(StaticModelResponse{.batchId = event.batchId,
                                       .requestId = event.requestId,
                                       .entityName = event.entityName,
                                       .gameObjectId = entt::to_integral(entityId)});
}

}
