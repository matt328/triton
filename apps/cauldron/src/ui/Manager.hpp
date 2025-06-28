#pragma once

#include "api/gw/editordata/EditorState.hpp"

namespace tr {
class IGuiCallbackRegistrar;
}

namespace ed {

struct AppLog;
class AssetTool;
class AssetViewer;
class EntityEditor;
class Menu;
class Properties;
class ApplicationController;

class Manager {
public:
  explicit Manager(std::shared_ptr<Menu> newAppMenu,
                   std::shared_ptr<AssetViewer> newAssetViewer,
                   std::shared_ptr<EntityEditor> newEntityEditor,
                   std::shared_ptr<Properties> newProperties,
                   std::shared_ptr<AssetTool> newAssetTool,
                   std::shared_ptr<tr::IGuiCallbackRegistrar> guiCallbackRegistrar,
                   std::shared_ptr<ApplicationController> newApplicationController);
  ~Manager();

  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  auto operator=(const Manager&) -> Manager& = delete;
  auto operator=(Manager&&) -> Manager& = delete;

  void render(const tr::EditorState& editorState);

private:
  ImFont* sauce = nullptr;
  std::shared_ptr<AppLog> appLog;
  std::shared_ptr<Menu> appMenu;
  std::shared_ptr<AssetViewer> assetViewer;
  std::shared_ptr<EntityEditor> entityEditor;
  std::shared_ptr<Properties> properties;
  std::shared_ptr<AssetTool> assetTool;
  std::shared_ptr<tr::IGuiCallbackRegistrar> guiCallbackRegistrar;
  std::shared_ptr<ApplicationController> applicationController;

  bool isReady = false;

  auto setupFonts() -> void;
};

}
