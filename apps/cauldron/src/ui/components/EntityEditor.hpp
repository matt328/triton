#pragma once

#include "IComponent.hpp"

namespace tr {
class IEventQueue;
class IGameWorldSystem;
}

namespace ed {

class EntityEditor : IComponent {
public:
  explicit EntityEditor(std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~EntityEditor();

  EntityEditor(const EntityEditor&) = delete;
  EntityEditor(EntityEditor&&) = delete;
  auto operator=(const EntityEditor&) -> EntityEditor& = delete;
  auto operator=(EntityEditor&&) -> EntityEditor& = delete;

  auto render(const tr::EditorState& editorState) -> void override;
  auto bindInput() -> void override;

  static constexpr auto ComponentName = "Entity Editor";

private:
  std::shared_ptr<tr::IEventQueue> eventQueue;

  struct StaticGameObjectDialogInfo {
    bool shouldShow;
    bool isOpen;
    std::string objectName;
    tr::FileAlias selectedModel;
  };

  StaticGameObjectDialogInfo staticDialogInfo{};

  struct AnimatedGameObjectDialogInfo {
    bool shouldShow;
    bool isOpen;
    std::string objectName;
    tr::FileAlias selectedModel;
    tr::FileAlias selectedSkeleton;
    tr::FileAlias selectedAnimation;
  };

  AnimatedGameObjectDialogInfo animatedDialogInfo{};

  auto renderStaticEntityDialog(const tr::EditorState& editorState) -> void;
  auto renderAnimatedGameObjectDialog(const tr::EditorState& editorState) -> void;
};

}
