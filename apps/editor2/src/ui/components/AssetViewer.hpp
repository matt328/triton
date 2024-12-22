#pragma once

#include "data/DataFacade.hpp"
#include "ui/components/DialogManager.hpp"

namespace ed {

class AssetViewer {
 public:
   AssetViewer(std::shared_ptr<DataFacade> newDataFacade,
               std::shared_ptr<DialogManager> newDialogManager);
   ~AssetViewer();

   AssetViewer(const AssetViewer&) = default;
   AssetViewer(AssetViewer&&) = delete;
   auto operator=(const AssetViewer&) -> AssetViewer& = default;
   auto operator=(AssetViewer&&) -> AssetViewer& = delete;

   void render();

 private:
   std::shared_ptr<DataFacade> dataFacade;
   std::shared_ptr<DialogManager> dialogManager;

   void createSkeletonDialog();
   void createAnimationDialog();
   void createModelDialog();
};

}
