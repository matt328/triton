#pragma once

#include "IComponent.hpp"

namespace tr {
class IEventQueue;
class IGameWorldSystem;
}

namespace ed {

struct TerrainData;

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
    int32_t objectCount;
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

  auto renderMenuBar() -> void;
  auto renderEntityList(const tr::EditorState& editorState) -> void;
  auto renderEntityDetailView(const tr::EditorState& editorState) -> void;

  auto renderTerrainControls(const TerrainData* terrainData) -> void;

  auto createTestTriangle() -> void;

  static auto renderOkCancelButtons(float buttonWidth = 80.f) -> std::pair<bool, bool>;

  template <typename T>
  auto renderAliasCombo(const char* label,
                        const std::unordered_map<std::string, T>& options,
                        T& selected) -> void {
    if (ImGui::BeginCombo(label, selected.alias.c_str())) {
      for (const auto& [key, option] : options) {
        bool isSelected = (selected == option);
        if (ImGui::Selectable(option.alias.c_str(), isSelected)) {
          selected = option;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }
};

}
