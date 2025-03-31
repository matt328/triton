#pragma once

namespace tr {
class IEventBus;
class IGameWorldSystem;
class IGameObjectProxy;
}

namespace ed {

class DataFacade;
class DialogManager;

class EntityEditor {
public:
  EntityEditor(std::shared_ptr<tr::IGameWorldSystem> newGameWorldSystem,
               std::shared_ptr<DataFacade> newDataFacade,
               std::shared_ptr<DialogManager> newDialogManager,
               std::shared_ptr<tr::IEventBus> newEventBus,
               std::shared_ptr<tr::IGameObjectProxy> newGameObjectProxy);
  ~EntityEditor();

  EntityEditor(const EntityEditor&) = delete;
  EntityEditor(EntityEditor&&) = delete;
  auto operator=(const EntityEditor&) -> EntityEditor& = delete;
  auto operator=(EntityEditor&&) -> EntityEditor& = delete;

  void render();

  static constexpr auto ComponentName = "Entity Editor";

private:
  std::shared_ptr<tr::IGameWorldSystem> gameWorldSystem;
  std::shared_ptr<DataFacade> dataFacade;
  std::shared_ptr<DialogManager> dialogManager;
  std::shared_ptr<tr::IEventBus> eventBus;
  std::shared_ptr<tr::IGameObjectProxy> gameObjectProxy;

  std::optional<std::string> selectedEntity{std::nullopt};

  void createAnimatedEntityDialog() const;
  void createStaticEntityDialog() const;
};

}
