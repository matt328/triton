#pragma once

#include "dialog/ModalDialog.hpp"

namespace ed {

class ModalDialog;

class DialogManager {
public:
  DialogManager() = default;
  ~DialogManager() = default;

  DialogManager(const DialogManager&) = default;
  DialogManager(DialogManager&&) = delete;
  auto operator=(const DialogManager&) -> DialogManager& = default;
  auto operator=(DialogManager&&) -> DialogManager& = delete;

  void render();
  void addDialog(const std::string& dialogName, std::unique_ptr<ModalDialog> dialog);
  void setOpen(const std::string& dialogName);
  void update();

private:
  std::unordered_map<std::string, std::unique_ptr<ModalDialog>> dialogMap;
};

}
