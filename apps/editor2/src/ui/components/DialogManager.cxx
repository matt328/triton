#include "DialogManager.hpp"

namespace ed::ui::cmp {

   void DialogManager::setOpen(const std::string& dialogName) {
      dialogMap.at(dialogName)->setOpen();
   }

   void DialogManager::update() {
      for (const auto& [name, dialog] : dialogMap) {
         dialog->checkShouldOpen();
      }
   }

}
