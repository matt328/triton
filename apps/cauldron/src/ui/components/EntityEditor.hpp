#pragma once

#include "IComponent.hpp"

namespace tr {
class IEventQueue;
class IGameWorldSystem;
}

namespace ed {

class DataFacade;
class DialogManager;

class EntityEditor : IComponent {
public:
  EntityEditor(std::shared_ptr<tr::IEventQueue> newEventQueue,
               std::shared_ptr<DialogManager> newDialogManager);
  ~EntityEditor();

  EntityEditor(const EntityEditor&) = delete;
  EntityEditor(EntityEditor&&) = delete;
  auto operator=(const EntityEditor&) -> EntityEditor& = delete;
  auto operator=(EntityEditor&&) -> EntityEditor& = delete;

  auto render(const tr::EditorState& editorState) -> void override;

  static constexpr auto ComponentName = "Entity Editor";

private:
  std::shared_ptr<tr::IEventQueue> eventQueue;
  std::shared_ptr<DialogManager> dialogManager;

  void createAnimatedEntityDialog() const;
  void createStaticEntityDialog() const;
};

}
