#pragma once

#include "IComponent.hpp"

namespace tr {
class IEventQueue;
}

namespace ed {

class DataFacade;
class FileDialog;
class Properties;
class DialogManager;

class Menu : public IComponent {
public:
  Menu(std::shared_ptr<DataFacade> newDataFacade,
       std::shared_ptr<Properties> newProperties,
       std::shared_ptr<DialogManager> newDialogManager,
       std::shared_ptr<tr::IEventQueue> newEventQueue);
  ~Menu();

  Menu(const Menu&) = delete;
  Menu(Menu&&) = delete;
  auto operator=(const Menu&) -> Menu& = delete;
  auto operator=(Menu&&) -> Menu& = delete;

  auto render(const tr::EditorContextData& uiState) -> void override;

  auto bindInput() -> void override;

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
  std::shared_ptr<tr::IEventQueue> eventQueue;

  /// This state should only be mutated/read from within the render method
  struct State {
    bool demoWindowVisible = false;
  };

  State state{};

  std::optional<std::filesystem::path> openFilePath;

  std::unique_ptr<FileDialog> projectSaveDialog;
  std::unique_ptr<FileDialog> projectOpenDialog;

  bool fullscreen{};
  bool enableWireframe{};

  std::function<void(void)> quitFn;
  std::function<void(void)> toggleFullscreenFn;
};
}
