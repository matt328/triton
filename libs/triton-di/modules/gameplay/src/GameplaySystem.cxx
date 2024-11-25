#include "GameplaySystem.hpp"
#include "gfx/ResourceManager.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "commands/CreateTestEntity.hpp"
#include "commands/CreateStaticEntity.hpp"
#include "commands/CreateCamera.hpp"
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
                                  std::shared_ptr<sys::TransformSystem> newTransformSystem,
                                  std::shared_ptr<sys::RenderDataSystem> newRenderDataSystem)
       : eventBus{std::move(newEventBus)},
         registry{std::move(newRegistry)},
         cameraSystem{std::move(newCameraSystem)},
         resourceManager{std::move(newResourceManager)},
         transformSystem{std::move(newTransformSystem)},
         renderDataSystem{std::move(newRenderDataSystem)} {
      Log.trace("Creating Gameplay System");

      auto& reg = registry->getRegistry();

      entityCreatedConnection =
          reg.on_construct<entt::entity>().connect<&GameplaySystem::entityCreated>(this);

      commandQueue = std::make_unique<
          CommandQueue<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&>>();

      eventBus->subscribe<SwapchainResized>([&](const SwapchainResized& event) {
         reg.ctx().insert_or_assign<cmp::WindowDimensions>(
             cmp::WindowDimensions{event.width, event.height});
      });

      // Forward
      actionSystem->mapSource(Source{Key::Up, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveForward);
      actionSystem->mapSource(Source{Key::W, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveForward);

      // Backward
      actionSystem->mapSource(Source{Key::Down, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveBackward);
      actionSystem->mapSource(Source{Key::S, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveBackward);
      // Left
      actionSystem->mapSource(Source{Key::Left, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeLeft);
      actionSystem->mapSource(Source{Key::A, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeLeft);
      // Right
      actionSystem->mapSource(Source{Key::Right, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeRight);
      actionSystem->mapSource(Source{Key::D, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeRight);
      // Look
      actionSystem->mapSource(Source{MouseInput::MOVE_X, SourceType::Float},
                              StateType::Range,
                              ActionType::LookHorizontal);
      actionSystem->mapSource(Source{MouseInput::MOVE_Y, SourceType::Float},
                              StateType::Range,
                              ActionType::LookVertical);
   }
   GameplaySystem::~GameplaySystem() {
      Log.trace("Destroying Gameplay System");
      entityCreatedConnection.release();
   }

   void GameplaySystem::update() {
      {
         ZoneNamedN(rd, "RenderData", true);
         renderData.objectData.clear();
         renderData.staticMeshData.clear();
         renderData.terrainMeshData.clear();
         renderData.skinnedMeshData.clear();
         renderData.animationData.clear();
         renderDataSystem->update(renderData);
      }
      transferHandler(renderData);
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

   auto GameplaySystem::createDefaultCamera() -> void {
      const auto [width, height] = registry->getRegistry().ctx().get<const cmp::WindowDimensions>();

      auto cameraInfo = cmp::CameraInfo{
          .width = width,
          .height = height,
          .fov = 60.f,
          .nearClip = 0.1f,
          .farClip = 10000.f,
          .position = glm::vec3{1.0f, 1.0f, 3.0f},
      };
      commandQueue->enqueue(std::make_unique<cmd::CreateCamera>(cameraInfo));
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
