#pragma once

#include "tr/IGameplaySystem.hpp"
#include "data/DataFacade.hpp"
#include "ui/components/DialogManager.hpp"

namespace ed::ui::cmp {

   class EntityEditor {
    public:
      EntityEditor(std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem,
                   std::shared_ptr<data::DataFacade> newDataFacade,
                   std::shared_ptr<DialogManager> newDialogManager);
      ~EntityEditor();

      EntityEditor(const EntityEditor&) = default;
      EntityEditor(EntityEditor&&) = delete;
      auto operator=(const EntityEditor&) -> EntityEditor& = default;
      auto operator=(EntityEditor&&) -> EntityEditor& = delete;

      void render();

    private:
      std::shared_ptr<tr::gp::IGameplaySystem> gameplaySystem;
      std::shared_ptr<data::DataFacade> dataFacade;
      std::shared_ptr<DialogManager> dialogManager;

      std::optional<std::string> selectedEntity{std::nullopt};

      void createAnimatedEntityDialog();
   };

}
