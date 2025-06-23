#pragma once

#include "IComponent.hpp"

namespace ed {

class DataFacade;
class DialogManager;
class Properties;

class AssetViewer : public IComponent {
public:
  AssetViewer(std::shared_ptr<DialogManager> newDialogManager,
              std::shared_ptr<Properties> newProperties);
  ~AssetViewer();

  AssetViewer(const AssetViewer&) = default;
  AssetViewer(AssetViewer&&) = delete;
  auto operator=(const AssetViewer&) -> AssetViewer& = default;
  auto operator=(AssetViewer&&) -> AssetViewer& = delete;

  auto render(const UIState& uiState) -> void override;

private:
  std::shared_ptr<DialogManager> dialogManager;
  std::shared_ptr<Properties> properties;

  void createSkeletonDialog();
  void createAnimationDialog();
  void createModelDialog();
};

}
