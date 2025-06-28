#include "EntityManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/fx/IStateBuffer.hpp"
#include "api/gw/editordata/EditorState.hpp"
#include "components/Renderable.hpp"
#include "components/Resources.hpp"
#include "components/Transform.hpp"
#include "components/EditorInfo.hpp"
#include "systems2/CameraHandler.hpp"
#include "systems2/FinalizerSystem.hpp"
#include "systems2/EditorSystem.hpp"
#include "api/gw/EditorStateBuffer.hpp"

namespace tr {

constexpr auto DefaultFOV = 60.f;
constexpr auto DefaultNearClip = 0.1f;
constexpr auto DefaultFarClip = 10000.f;
constexpr auto DefaultPosition = glm::vec3{0.f, 0.f, 5.f};

EntityManager::EntityManager(std::shared_ptr<IEventQueue> newEventQueue,
                             std::shared_ptr<IStateBuffer> newStateBuffer,
                             std::shared_ptr<EditorStateBuffer> newEditorStateBuffer)
    : eventQueue{std::move(newEventQueue)},
      stateBuffer{std::move(newStateBuffer)},
      editorStateBuffer{std::move(newEditorStateBuffer)} {
  registry = std::make_unique<entt::registry>();
  registry->ctx().emplace<EditorContextData>();
  Log.trace("Created EntityManager");

  eventQueue->subscribe<Action>(
      [this](const Action& event) { CameraHandler::handleAction(event, *registry); });

  eventQueue->subscribe<SwapchainCreated>(
      [this](const SwapchainCreated& event) { renderAreaCreated(event); });

  eventQueue->subscribe<SwapchainResized>(
      [this](const SwapchainResized& event) { renderAreaResized(event); });

  eventQueue->subscribe<tr::AddSkeleton>(
      [this](const tr::AddSkeleton& event) { addSkeleton(event.name, event.fileName); });
  eventQueue->subscribe<tr::AddAnimation>(
      [this](const tr::AddAnimation& event) { addAnimation(event.name, event.fileName); });
  eventQueue->subscribe<tr::AddModel>(
      [this](const tr::AddModel& event) { addModel(event.name, event.fileName); });

  eventQueue->subscribe<tr::CreateStaticGameObject>(
      [this](const tr::CreateStaticGameObject& event) {
        createStaticGameObject(event.entityName, event.geometryHandle, event.gameObjectData);
      });
}

EntityManager::~EntityManager() {
  Log.trace("Destroying EntityManager");
}

auto EntityManager::update() -> void {
  Timestamp currentTime = std::chrono::steady_clock::now();
  SimState writeState = SimState{1};
  FinalizerSystem::update(*registry, writeState, currentTime);
  stateBuffer->pushState(writeState, currentTime);

  editorStateBuffer->pushState(EditorSystem::update(*registry), currentTime);
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

auto EntityManager::createStaticGameObject(std::string entityName,
                                           Handle<Geometry> geometryHandle,
                                           const GameObjectData& gameObjectData) -> void {
  ZoneScoped;
  Log.trace("EntityManager creating static gameobject, geometryHandle={}", geometryHandle.id);

  auto entityId = registry->create();
  registry->emplace<Renderable>(entityId, std::vector<Handle<Geometry>>{geometryHandle});
  registry->emplace<Transform>(entityId);
  registry->emplace<GameObjectData>(entityId, gameObjectData);
  auto& ctxData = registry->ctx().get<EditorContextData>();
  ctxData.scene.objectNameMap.emplace(entityName, static_cast<tr::GameObjectId>(entityId));
}

auto EntityManager::addSkeleton(std::string name, std::string filename) -> void {
  auto& editorData = registry->ctx().get<EditorContextData>();
  editorData.assets.skeletons.emplace(
      name,
      FileAlias{.alias = name, .filePath = std::filesystem::path{filename}});
}

auto EntityManager::addAnimation(std::string name, std::string filename) -> void {
  auto& editorData = registry->ctx().get<EditorContextData>();
  editorData.assets.animations.emplace(
      name,
      FileAlias{.alias = name, .filePath = std::filesystem::path{filename}});
}

auto EntityManager::addModel(std::string name, std::string filename) -> void {
  Log.trace("addModel name={}, filename={}", name, filename);
  auto& editorData = registry->ctx().get<EditorContextData>();
  editorData.assets.models.emplace(
      name,
      FileAlias{.alias = name, .filePath = std::filesystem::path{filename}});
}

}
