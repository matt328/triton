#pragma once

#include "IComponent.hpp"

namespace bk {
class Preferences;
}

namespace ed {

class DataFacade;

class AssetViewer : public IComponent {
public:
  AssetViewer(std::shared_ptr<bk::Preferences> newPreferences,
              std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~AssetViewer();

  AssetViewer(const AssetViewer&) = default;
  AssetViewer(AssetViewer&&) = delete;
  auto operator=(const AssetViewer&) -> AssetViewer& = default;
  auto operator=(AssetViewer&&) -> AssetViewer& = delete;

  auto render(const tr::EditorState& editorState) -> void override;
  auto bindInput() -> void override;

private:
  std::shared_ptr<bk::Preferences> preferences;
  std::shared_ptr<tr::IEventQueue> eventQueue;

  struct AliasDialogInfo {
    bool shouldShow = false;
    bool isOpen = false;
    tr::FileAlias alias;
  };

  AliasDialogInfo modelAliasInfo;
  AliasDialogInfo skeletonAliasInfo;
  AliasDialogInfo animationAliasInfo;

  auto renderModelDialog() -> void;
  auto renderSkeletonDialog() -> void;
  auto renderAnimationDialog() -> void;
};

}
