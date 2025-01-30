#pragma once

#include "Properties.hpp"
#include "data/DataFacade.hpp"
#include "tr/IEventBus.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/FileDialog.hpp"

namespace ed {

class Menu {
public:
  explicit Menu(std::shared_ptr<DataFacade> newDataFacade,
                std::shared_ptr<Properties> newProperties,
                std::shared_ptr<DialogManager> newDialogManager,
                std::shared_ptr<tr::IEventBus> newEventBus);
  ~Menu();

  Menu(const Menu&) = delete;
  Menu(Menu&&) = delete;
  auto operator=(const Menu&) -> Menu& = delete;
  auto operator=(Menu&&) -> Menu& = delete;

  void render();

  void setQuitFn(const std::function<void(void)>& newQuitFn) {
    quitFn = newQuitFn;
  }

  void setFullscreenFn(std::function<void(void)>& newFullscreenFn) {
    toggleFullscreenFn = newFullscreenFn;
  }

private:
  std::shared_ptr<DataFacade> dataFacade;
  std::shared_ptr<Properties> properties;
  std::shared_ptr<DialogManager> dialogManager;
  std::shared_ptr<tr::IEventBus> eventBus;

  std::optional<std::filesystem::path> openFilePath;

  std::unique_ptr<FileDialog> projectSaveDialog;
  std::unique_ptr<FileDialog> projectOpenDialog;

  bool fullscreen{};
  bool enableWireframe{};

  std::function<void(void)> quitFn;
  std::function<void(void)> toggleFullscreenFn;
};
}
