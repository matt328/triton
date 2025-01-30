#pragma once

#include "TaskQueue.hpp"
#include "components/AppLog.hpp"
#include "data/DataFacade.hpp"
#include "ui/components/AssetViewer.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/EntityEditor.hpp"
#include "ui/components/Menu.hpp"

namespace ed {

class Manager {
public:
  explicit Manager(std::shared_ptr<Menu> newAppMenu,
                   std::shared_ptr<AssetViewer> newAssetViewer,
                   std::shared_ptr<DialogManager> newDialogManager,
                   std::shared_ptr<EntityEditor> newEntityEditor,
                   std::shared_ptr<TaskQueue> newTaskQueue,
                   std::shared_ptr<Properties> newProperties,
                   std::shared_ptr<DataFacade> newDataFacade);
  ~Manager();

  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  auto operator=(const Manager&) -> Manager& = delete;
  auto operator=(Manager&&) -> Manager& = delete;

  void render();

private:
  ImFont* sauce = nullptr;
  std::shared_ptr<AppLog> appLog;
  std::shared_ptr<Menu> appMenu;
  std::shared_ptr<AssetViewer> assetViewer;
  std::shared_ptr<DialogManager> dialogManager;
  std::shared_ptr<EntityEditor> entityEditor;
  std::shared_ptr<TaskQueue> taskQueue;
  std::shared_ptr<Properties> properties;
  std::shared_ptr<DataFacade> dataFacade;

  auto setupFonts() -> void;
};

}
