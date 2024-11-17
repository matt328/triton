#include "DialogManager.hpp"

namespace ed::ui::cmp {

   void DialogManager::render() {
      for (const auto& [name, dialog] : dialogMap) {
         dialog->render();
      }
   }

   void DialogManager::addDialog(const std::string& dialogName,
                                 std::unique_ptr<ModalDialog> dialog) {
      dialogMap[dialogName] = std::move(dialog);
   }

   void DialogManager::setOpen(const std::string& dialogName) {
      dialogMap.at(dialogName)->setOpen();
   }

   void DialogManager::update() {
      for (const auto& [name, dialog] : dialogMap) {
         dialog->checkShouldOpen();
      }
   }

}
