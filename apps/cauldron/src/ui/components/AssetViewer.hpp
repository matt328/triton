#pragma once

#include "IComponent.hpp"

namespace ed {

class DataFacade;
class Properties;

class AssetViewer : public IComponent {
public:
  AssetViewer(std::shared_ptr<Properties> newProperties,
              std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~AssetViewer();

  AssetViewer(const AssetViewer&) = default;
  AssetViewer(AssetViewer&&) = delete;
  auto operator=(const AssetViewer&) -> AssetViewer& = default;
  auto operator=(AssetViewer&&) -> AssetViewer& = delete;

  auto render(const tr::EditorState& uiState) -> void override;
  auto bindInput() -> void override;

private:
  std::shared_ptr<Properties> properties;
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
