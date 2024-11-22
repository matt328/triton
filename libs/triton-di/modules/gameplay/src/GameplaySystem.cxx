#include "GameplaySystem.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "commands/CreateTestEntity.hpp"
#include "tr/Events.hpp"
#include "tr/IGameplaySystem.hpp"
#include "gp/components/Resources.hpp"
#include <entt/entity/fwd.hpp>

namespace tr::gp {

   GameplaySystem::GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem,
                                  std::shared_ptr<IEventBus> newEventBus,
                                  std::shared_ptr<Registry> newRegistry,
                                  std::shared_ptr<sys::CameraSystem> newCameraSystem)
       : eventBus{std::move(newEventBus)},
         registry{std::move(newRegistry)},
         cameraSystem{std::move(newCameraSystem)} {
      Log.trace("Creating Gameplay System");

      registry->getRegistry().on_construct<entt::entity>().connect<&GameplaySystem::entityCreated>(
          this);

      commandQueue = std::make_unique<CommandQueue<entt::registry>>();

      eventBus->subscribe<SwapchainResized>([&](const SwapchainResized& event) {
         auto& reg = registry->getRegistry();
         reg.ctx().insert_or_assign<cmp::WindowDimensions>(
             cmp::WindowDimensions{event.height, event.width});
      });

      // Forward
      actionSystem->mapSource(Source{cm::Key::Up, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveForward);
      actionSystem->mapSource(Source{cm::Key::W, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveForward);

      // Backward
      actionSystem->mapSource(Source{cm::Key::Down, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveBackward);
      actionSystem->mapSource(Source{cm::Key::S, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveBackward);
      // Left
      actionSystem->mapSource(Source{cm::Key::Left, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeLeft);
      actionSystem->mapSource(Source{cm::Key::A, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeLeft);
      // Right
      actionSystem->mapSource(Source{cm::Key::Right, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeRight);
      actionSystem->mapSource(Source{cm::Key::D, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeRight);
      // Look
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_X, SourceType::Float},
                              tr::StateType::Range,
                              tr::ActionType::LookHorizontal);
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_Y, SourceType::Float},
                              tr::StateType::Range,
                              tr::ActionType::LookVertical);
   }

   void GameplaySystem::update() {
   }

   void GameplaySystem::fixedUpdate() {
      std::unique_lock lock{registryMutex};
      commandQueue->processCommands(registry->getRegistry());

      {
         ZoneNamedN(camZone, "CameraSystem", true);
         cameraSystem->fixedUpdate();
      }
   }

   void GameplaySystem::setRenderDataTransferHandler(const RenderDataTransferHandler& handler) {
      this->transferHandler = handler;
   }

   auto GameplaySystem::createStaticModelEntity([[maybe_unused]] std::string filename)
       -> cm::EntityType {
      return static_cast<cm::EntityType>(1);
   }

   auto GameplaySystem::createAnimatedModelEntity(const AnimatedModelData& modelData)
       -> cm::EntityType {
      Log.trace("gameplaySystem createAnimatedModelEntity: {0}", modelData);

      // have gfx create the resources and return a handle.

      // use the handle(s) to create an entity in the ECS

      return static_cast<cm::EntityType>(1);
   }

   auto GameplaySystem::createTerrain() -> cm::EntityType {
      return static_cast<cm::EntityType>(1);
   }

   auto GameplaySystem::createDefaultCamera() -> cm::EntityType {
      return static_cast<cm::EntityType>(1);
   }

   auto GameplaySystem::createTestEntity(std::string_view name) -> void {
      commandQueue->enqueue(std::make_unique<CreateTestEntityCommand>(name));
   }

   void GameplaySystem::entityCreated([[maybe_unused]] entt::registry& reg,
                                      [[maybe_unused]] entt::entity entity) {
      Log.trace("Entity Created: {}", static_cast<uint32_t>(entity));
      eventBus->emit(EntityCreated{entity});
   }
}
