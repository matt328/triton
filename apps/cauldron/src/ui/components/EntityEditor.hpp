#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"
#include "data/DataFacade.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/editors/TransformInspector.hpp"
#include <entt/entity/fwd.hpp>

namespace ed {

using ComponentInspectorFn = void (*)(entt::registry, tr::EntityType);

class EntityEditor {
public:
  EntityEditor(std::shared_ptr<tr::IGameplaySystem> newGameplaySystem,
               std::shared_ptr<DataFacade> newDataFacade,
               std::shared_ptr<DialogManager> newDialogManager,
               std::shared_ptr<tr::IEventBus> newEventBus,
               std::shared_ptr<entt::registry> newRegistry);
  ~EntityEditor();

  EntityEditor(const EntityEditor&) = delete;
  EntityEditor(EntityEditor&&) = delete;
  auto operator=(const EntityEditor&) -> EntityEditor& = delete;
  auto operator=(EntityEditor&&) -> EntityEditor& = delete;

  void render();

private:
  std::shared_ptr<tr::IGameplaySystem> gameplaySystem;
  std::shared_ptr<DataFacade> dataFacade;
  std::shared_ptr<DialogManager> dialogManager;
  std::shared_ptr<tr::IEventBus> eventBus;
  std::shared_ptr<entt::registry> registry;

  std::optional<tr::EntityType> selectedEntity{std::nullopt};

  std::unique_ptr<TransformInspector> transformInspector;

  void createAnimatedEntityDialog() const;
  void createStaticEntityDialog() const;
};

}
