#pragma once

#include "components/AppLog.hpp"
#include "ui/components/AssetViewer.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/EntityEditor.hpp"
#include "ui/components/Menu.hpp"

namespace ed::ui {

   class Manager {
    public:
      explicit Manager(std::shared_ptr<cmp::Menu> newAppMenu,
                       std::shared_ptr<cmp::AssetViewer> newAssetViewer,
                       std::shared_ptr<cmp::DialogManager> newDialogManager,
                       std::shared_ptr<cmp::EntityEditor> newEntityEditor);
      ~Manager();

      Manager(const Manager&) = delete;
      Manager(Manager&&) = delete;
      auto operator=(const Manager&) -> Manager& = delete;
      auto operator=(Manager&&) -> Manager& = delete;

      void render();

    private:
      ImFont* sauce = nullptr;
      std::shared_ptr<cmp::AppLog> appLog;
      std::shared_ptr<cmp::Menu> appMenu;
      std::shared_ptr<cmp::AssetViewer> assetViewer;
      std::shared_ptr<cmp::DialogManager> dialogManager;
      std::shared_ptr<cmp::EntityEditor> entityEditor;

      auto setupFonts() -> void;
   };

}
