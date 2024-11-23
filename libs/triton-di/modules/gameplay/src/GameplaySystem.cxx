#include "GameplaySystem.hpp"
#include "gfx/ResourceManager.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "commands/CreateTestEntity.hpp"
#include "commands/CreateStaticEntity.hpp"
#include "tr/Events.hpp"
#include "tr/IGameplaySystem.hpp"
#include "gp/components/Resources.hpp"
#include <entt/entity/fwd.hpp>

namespace tr::gp {

   GameplaySystem::GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem,
                                  std::shared_ptr<IEventBus> newEventBus,
                                  std::shared_ptr<Registry> newRegistry,
                                  std::shared_ptr<sys::CameraSystem> newCameraSystem,
                                  std::shared_ptr<gfx::ResourceManager> newResourceManager,
                                  std::shared_ptr<sys::TransformSystem> newTransformSystem)
       : eventBus{std::move(newEventBus)},
         registry{std::move(newRegistry)},
         cameraSystem{std::move(newCameraSystem)},
         resourceManager{std::move(newResourceManager)},
         transformSystem{std::move(newTransformSystem)} {
      Log.trace("Creating Gameplay System");

      auto& reg = registry->getRegistry();

      entityCreatedConnection =
          reg.on_construct<entt::entity>().connect<&GameplaySystem::entityCreated>(this);

      commandQueue = std::make_unique<
          CommandQueue<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&>>();

      eventBus->subscribe<SwapchainResized>([&](const SwapchainResized& event) {
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
   GameplaySystem::~GameplaySystem() {
      Log.trace("Destroying Gameplay System");
      entityCreatedConnection.release();
   }

   void GameplaySystem::update() {
   }

   void GameplaySystem::fixedUpdate() {
      std::unique_lock const lock{registryMutex};
      commandQueue->processCommands(registry->getRegistry(), resourceManager);
      {
         ZoneNamedN(camZone, "CameraSystem", true);
         cameraSystem->fixedUpdate();
      }
      {
         ZoneNamedN(xformZone, "Transform", true);
         transformSystem->update();
      }
   }

   void GameplaySystem::setRenderDataTransferHandler(const RenderDataTransferHandler& handler) {
      this->transferHandler = handler;
   }

   auto GameplaySystem::createStaticModelEntity(std::string filename,
                                                const std::string_view entityName) -> void {
      commandQueue->enqueue(
          std::make_unique<cmd::CreateStaticEntityCommand>(filename, entityName.data()));
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
                                      [[maybe_unused]] entt::entity entity) const {
      Log.trace("Entity Created: {}", static_cast<uint32_t>(entity));
      eventBus->emit(EntityCreated{entity});
   }
}
