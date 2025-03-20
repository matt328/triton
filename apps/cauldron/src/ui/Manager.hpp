#pragma once

namespace tr {
class ITaskQueue;
}

namespace ed {

struct AppLog;
class AssetTool;
class DataFacade;
class AssetViewer;
class DialogManager;
class EntityEditor;
class Menu;
class Properties;

class Manager {
public:
  explicit Manager(std::shared_ptr<Menu> newAppMenu,
                   std::shared_ptr<AssetViewer> newAssetViewer,
                   std::shared_ptr<DialogManager> newDialogManager,
                   std::shared_ptr<EntityEditor> newEntityEditor,
                   std::shared_ptr<tr::ITaskQueue> newTaskQueue,
                   std::shared_ptr<Properties> newProperties,
                   std::shared_ptr<DataFacade> newDataFacade,
                   std::shared_ptr<AssetTool> newAssetTool);
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
  std::shared_ptr<tr::ITaskQueue> taskQueue;
  std::shared_ptr<Properties> properties;
  std::shared_ptr<DataFacade> dataFacade;
  std::shared_ptr<AssetTool> assetTool;

  auto setupFonts() -> void;
};

}
