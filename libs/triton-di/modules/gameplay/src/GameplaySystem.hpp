#pragma once

#include "gp/Registry.hpp"
#include "systems/CameraSystem.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"
#include "gp/action/IActionSystem.hpp"
#include "CommandQueue.hpp"
#include <entt/entity/fwd.hpp>

namespace tr::gp {
   class GameplaySystem : public IGameplaySystem {
    public:
      explicit GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem,
                              std::shared_ptr<IEventBus> newEventBus,
                              std::shared_ptr<Registry> newRegistry,
                              std::shared_ptr<sys::CameraSystem> newCameraSystem);
      ~GameplaySystem() override = default;

      GameplaySystem(const GameplaySystem&) = delete;
      GameplaySystem(GameplaySystem&&) = delete;
      auto operator=(const GameplaySystem&) -> GameplaySystem& = delete;
      auto operator=(GameplaySystem&&) -> GameplaySystem& = delete;

      void update() override;
      void fixedUpdate() override;

      void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) override;

      auto createStaticModelEntity(std::string filename) -> cm::EntityType override;
      auto createAnimatedModelEntity(const AnimatedModelData& modelData) -> cm::EntityType override;
      auto createTerrain() -> cm::EntityType override;
      auto createDefaultCamera() -> cm::EntityType override;
      auto createTestEntity(std::string_view name) -> void override;

    private:
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<Registry> registry;
      std::shared_ptr<sys::CameraSystem> cameraSystem;

      RenderDataTransferHandler transferHandler;
      std::shared_mutex registryMutex{};
      std::unique_ptr<CommandQueue<entt::registry>> commandQueue;

      void entityCreated(entt::registry&, entt::entity);
   };
}
