#include "EntityManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/fx/IStateBuffer.hpp"
#include "api/gw/editordata/EditorState.hpp"
#include "api/gw/editordata/Project.hpp"
#include "components/Renderable.hpp"
#include "components/Resources.hpp"
#include "components/Transform.hpp"
#include "components/EditorInfo.hpp"
#include "systems2/CameraHandler.hpp"
#include "systems2/FinalizerSystem.hpp"
#include "systems2/EditorSystem.hpp"
#include "api/gw/EditorStateBuffer.hpp"
#include <cereal/archives/binary.hpp>

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

  eventQueue->subscribe<Action>([this](const std::shared_ptr<Action>& event) {
    CameraHandler::handleAction(event, *registry);
  });

  eventQueue->subscribe<SwapchainCreated>(
      [this](const std::shared_ptr<SwapchainCreated>& event) { renderAreaCreated(event); });

  eventQueue->subscribe<SwapchainResized>(
      [this](const std::shared_ptr<SwapchainResized>& event) { renderAreaResized(event); });

  eventQueue->subscribe<tr::AddSkeleton>([this](const std::shared_ptr<tr::AddSkeleton>& event) {
    addSkeleton(event->name, event->fileName, event->fromFile);
  });
  eventQueue->subscribe<tr::AddAnimation>([this](const std::shared_ptr<tr::AddAnimation>& event) {
    addAnimation(event->name, event->fileName, event->fromFile);
  });
  eventQueue->subscribe<tr::AddModel>([this](const std::shared_ptr<tr::AddModel>& event) {
    addModel(event->name, event->fileName, event->fromFile);
  });

  eventQueue->subscribe<tr::SelectEntity>(
      [this](const std::shared_ptr<tr::SelectEntity>& event) { selectEntity(event->entityId); });

  eventQueue->subscribe<tr::SaveProject>(
      [this](const std::shared_ptr<tr::SaveProject>& event) { saveProject(event->filePath); });

  eventQueue->subscribe<tr::LoadProject>(
      [this](const std::shared_ptr<tr::LoadProject>& event) { loadProject(event->filePath); });

  eventQueue->subscribe<tr::CreateStaticGameObject>(
      [this](const std::shared_ptr<tr::CreateStaticGameObject>& event) {
        createStaticGameObject(event->entityName,
                               event->geometryHandle,
                               event->gameObjectData,
                               event->textureHandle);
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

auto EntityManager::renderAreaCreated(const std::shared_ptr<SwapchainCreated>& event) -> void {
  registry->ctx().insert_or_assign<WindowDimensions>(
      WindowDimensions{.width = event->width, .height = event->height});
  createDefaultCamera();
}

auto EntityManager::renderAreaResized(const std::shared_ptr<SwapchainResized>& event) -> void {
  registry->ctx().insert_or_assign<WindowDimensions>(
      WindowDimensions{.width = event->width, .height = event->height});
}

auto EntityManager::createDefaultCamera() -> void {
  auto cameraInfo = CameraInfo{
      .fov = DefaultFOV,
      .nearClip = DefaultNearClip,
      .farClip = DefaultFarClip,
      .position = DefaultPosition,
  };
  auto& [width, height] = registry->ctx().get<WindowDimensions>();

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
                                           const GameObjectData& gameObjectData,
                                           std::optional<Handle<TextureTag>> textureHandle)
    -> void {
  ZoneScoped;
  Log.trace("EntityManager creating static gameobject name={}, geometryHandle={}",
            entityName,
            geometryHandle.id);

  auto entityId = registry->create();
  auto textureHandles = std::vector<Handle<TextureTag>>{};
  if (textureHandle) {
    textureHandles.push_back(*textureHandle);
  }
  registry->emplace<Renderable>(entityId,
                                std::vector<Handle<Geometry>>{geometryHandle},
                                textureHandles);
  registry->emplace<Transform>(entityId,
                               Transform{
                                   .rotation = gameObjectData.orientation.rotation,
                                   .position = gameObjectData.orientation.position,
                                   .scale = {1.f, 1.f, 1.f},
                               });
  registry->emplace<GameObjectData>(entityId, gameObjectData);
  auto& ctxData = registry->ctx().get<EditorContextData>();
  ctxData.scene.objectNameMap.emplace(entityName, static_cast<tr::GameObjectId>(entityId));
}

auto EntityManager::addSkeleton(std::string name, std::string filename, bool fromFile) -> void {
  auto& editorData = registry->ctx().get<EditorContextData>();
  editorData.assets.skeletons.emplace(
      name,
      FileAlias{.alias = name, .filePath = std::filesystem::path{filename}});
  if (!fromFile) {
    editorData.saved = false;
  }
}

auto EntityManager::addAnimation(std::string name, std::string filename, bool fromFile) -> void {
  auto& editorData = registry->ctx().get<EditorContextData>();
  editorData.assets.animations.emplace(
      name,
      FileAlias{.alias = name, .filePath = std::filesystem::path{filename}});
  if (!fromFile) {
    editorData.saved = false;
  }
}

auto EntityManager::addModel(std::string name, std::string filename, bool fromFile) -> void {
  Log.trace("addModel name={}, filename={}", name, filename);
  auto& editorData = registry->ctx().get<EditorContextData>();
  editorData.assets.models.emplace(
      name,
      FileAlias{.alias = name, .filePath = std::filesystem::path{filename}});
  if (!fromFile) {
    editorData.saved = false;
  }
}

auto EntityManager::selectEntity(std::optional<std::string> entityName) -> void {
  auto& contextData = registry->ctx().get<EditorContextData>();
  contextData.selectedEntity = std::move(entityName);
}

auto EntityManager::saveProject(const std::filesystem::path& filePath) -> void {
  auto& editorData = registry->ctx().get<EditorContextData>();

  const auto view = registry->view<GameObjectData>();

  auto gameObjects = std::unordered_map<std::string, GameObjectData>{};

  for (const auto& [entity, gameObjectData] : view.each()) {
    gameObjects.emplace(gameObjectData.name, gameObjectData);
  }

  auto project = tr::Project{
      .skeletons = editorData.assets.skeletons,
      .animations = editorData.assets.animations,
      .models = editorData.assets.models,
      .gameObjects = gameObjects,
  };

  auto os = std::ofstream(filePath, std::ios::binary);
  cereal::BinaryOutputArchive output(os);
  output(project);
  Log.info("Wrote binary output file to {0}", filePath.string());
  editorData.saved = true;
}

auto EntityManager::loadProject(const std::filesystem::path& filePath) -> void {
  auto is = std::ifstream(filePath, std::ios::binary);
  cereal::BinaryInputArchive input(is);
  auto project = Project{};
  input(project);

  for (const auto& animation : project.animations) {
    eventQueue->emit(tr::AddAnimation{.name = animation.second.alias,
                                      .fileName = animation.second.filePath.string(),
                                      .fromFile = true});
  }

  for (const auto& skeleton : project.skeletons) {
    eventQueue->emit(tr::AddSkeleton{.name = skeleton.second.alias,
                                     .fileName = skeleton.second.filePath.string(),
                                     .fromFile = true});
  }

  for (const auto& model : project.models) {
    eventQueue->emit(tr::AddModel{.name = model.second.alias,
                                  .fileName = model.second.filePath.string(),
                                  .fromFile = true});
  }

  for (const auto& gameObject : project.gameObjects | std::ranges::views::values) {
    const auto addStaticModel = AddStaticModel{.name = gameObject.name,
                                               .orientation = gameObject.orientation,
                                               .modelName = gameObject.modelName,
                                               .fromFile = true};
    eventQueue->emit(addStaticModel);
  }
}

}
