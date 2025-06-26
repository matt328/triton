#include "DialogManager.hpp"

namespace ed {

void DialogManager::render() {
  for (const auto& [name, dialog] : dialogMap) {
    dialog->render();
  }
}

void DialogManager::addDialog(const std::string& dialogName, std::unique_ptr<ModalDialog> dialog) {
  dialogMap[dialogName] = std::move(dialog);
}

void DialogManager::setOpen(const std::string& dialogName,
                            const DialogRenderContext& renderContext) {
  if (dialogMap.contains(dialogName)) {
    dialogMap.at(dialogName)->setOpen(renderContext);
  } else {
    Log.warn("No dialog registered with name: {}", dialogName);
  }
}

void DialogManager::update() {
  for (const auto& [name, dialog] : dialogMap) {
    dialog->checkShouldOpen();
  }
}

}
