#pragma once

namespace tr {
class IEventQueue;
class IGameWorldSystem;
}

namespace ed {

class DataFacade;
class DialogManager;

class EntityEditor {
public:
  EntityEditor(std::shared_ptr<tr::IEventQueue> newEventQueue,
               std::shared_ptr<DataFacade> newDataFacade,
               std::shared_ptr<DialogManager> newDialogManager);
  ~EntityEditor();

  EntityEditor(const EntityEditor&) = delete;
  EntityEditor(EntityEditor&&) = delete;
  auto operator=(const EntityEditor&) -> EntityEditor& = delete;
  auto operator=(EntityEditor&&) -> EntityEditor& = delete;

  void render();

  static constexpr auto ComponentName = "Entity Editor";

private:
  std::shared_ptr<tr::IEventQueue> eventQueue;
  std::shared_ptr<DataFacade> dataFacade;
  std::shared_ptr<DialogManager> dialogManager;

  std::optional<std::string> selectedEntity{std::nullopt};

  void createAnimatedEntityDialog() const;
  void createStaticEntityDialog() const;
};

}
