#pragma once

#include "api/fx/Events.hpp"
#include "gw/IEntityManager.hpp"

namespace tr {

class IEventQueue;
class FinalizerSystem;
class IStateBuffer;
class FrameState;
class CameraHandler;
class EditorStateBuffer;

class EntityManager : public IEntityManager {
public:
  explicit EntityManager(std::shared_ptr<IEventQueue> newEventQueue,
                         std::shared_ptr<IStateBuffer> newStateBuffer,
                         std::shared_ptr<EditorStateBuffer> newEditorStateBuffer);
  ~EntityManager() override;

  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  auto operator=(const EntityManager&) -> EntityManager& = delete;
  auto operator=(EntityManager&&) -> EntityManager& = delete;

  auto update() -> void override;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IStateBuffer> stateBuffer;
  std::shared_ptr<EditorStateBuffer> editorStateBuffer;

  std::unique_ptr<entt::registry> registry;

  auto renderAreaCreated(const SwapchainCreated& event) -> void;
  auto renderAreaResized(const SwapchainResized& event) -> void;
  auto registerStaticModel(const StaticModelUploaded& event) -> void;
  auto createDefaultCamera() -> void;

  auto addSkeleton(std::string name, std::string filename, bool fromFile) -> void;
  auto addAnimation(std::string name, std::string filename, bool fromFile) -> void;
  auto addModel(std::string name, std::string filename, bool fromFile) -> void;
  auto saveProject(const std::filesystem::path& filePath) -> void;
  auto loadProject(const std::filesystem::path& filePath) -> void;
  auto selectEntity(std::optional<std::string> entityName) -> void;

  auto createStaticGameObject(std::string entityName,
                              Handle<Geometry> geometryHandle,
                              const GameObjectData& gameObjectData) -> void;
};

}
