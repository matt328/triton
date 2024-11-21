#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"
#include "data/DataFacade.hpp"
#include "ui/components/DialogManager.hpp"
#include <entt/entity/fwd.hpp>

namespace ed::ui::cmp {

   using ComponentInspectorFn = void (*)(entt::registry, tr::cm::EntityType);

   class EntityEditor {
    public:
      EntityEditor(std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem,
                   std::shared_ptr<data::DataFacade> newDataFacade,
                   std::shared_ptr<DialogManager> newDialogManager,
                   std::shared_ptr<tr::IEventBus> newEventBus);
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
      std::shared_ptr<tr::IEventBus> eventBus;

      std::optional<tr::cm::EntityType> selectedEntity{std::nullopt};

      void createAnimatedEntityDialog() const;
   };

}
