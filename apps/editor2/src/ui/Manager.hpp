#pragma once

#include "TaskQueue.hpp"
#include "components/AppLog.hpp"
#include "data/DataFacade.hpp"
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
                       std::shared_ptr<cmp::EntityEditor> newEntityEditor,
                       std::shared_ptr<TaskQueue> newTaskQueue,
                       std::shared_ptr<Properties> newProperties,
                       std::shared_ptr<data::DataFacade> newDataFacade);
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
      std::shared_ptr<TaskQueue> taskQueue;
      std::shared_ptr<Properties> properties;
      std::shared_ptr<data::DataFacade> dataFacade;

      auto setupFonts() -> void;
   };

}
