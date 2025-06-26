#pragma once

#include "IComponent.hpp"

namespace ed {

class DataFacade;
class DialogManager;
class Properties;

class AssetViewer : public IComponent {
public:
  AssetViewer(std::shared_ptr<DialogManager> newDialogManager,
              std::shared_ptr<Properties> newProperties,
              std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~AssetViewer();

  AssetViewer(const AssetViewer&) = default;
  AssetViewer(AssetViewer&&) = delete;
  auto operator=(const AssetViewer&) -> AssetViewer& = default;
  auto operator=(AssetViewer&&) -> AssetViewer& = delete;

  auto render(const tr::EditorState& uiState) -> void override;
  auto bindInput() -> void override;

private:
  std::shared_ptr<DialogManager> dialogManager;
  std::shared_ptr<Properties> properties;
  std::shared_ptr<tr::IEventQueue> eventQueue;

  void createSkeletonDialog();
  void createAnimationDialog();
  void createModelDialog();
};

}
