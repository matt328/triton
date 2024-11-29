#pragma once

#include "gfx/ResourceManager.hpp"
#include "gp/Registry.hpp"
#include "systems/CameraSystem.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"
#include "gp/action/IActionSystem.hpp"
#include "CommandQueue.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/RenderDataSystem.hpp"

#include <entt/entity/fwd.hpp>
#include <systems/AnimationSystem.hpp>

namespace tr::gp {
   namespace sys {
      class RenderDataSystem;
   }

   class GameplaySystem final : public IGameplaySystem {
    public:
      explicit GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem,
                              std::shared_ptr<IEventBus> newEventBus,
                              std::shared_ptr<Registry> newRegistry,
                              std::shared_ptr<sys::CameraSystem> newCameraSystem,
                              std::shared_ptr<gfx::ResourceManager> newResourceManager,
                              std::shared_ptr<sys::TransformSystem> newTransformSystem,
                              std::shared_ptr<sys::RenderDataSystem> newRenderDataSystem,
                              std::shared_ptr<sys::AnimationSystem> newAnimationSystem);
      ~GameplaySystem() override;

      GameplaySystem(const GameplaySystem&) = delete;
      GameplaySystem(GameplaySystem&&) = delete;
      auto operator=(const GameplaySystem&) -> GameplaySystem& = delete;
      auto operator=(GameplaySystem&&) -> GameplaySystem& = delete;

      void update() override;
      void fixedUpdate() override;

      void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) override;

      auto createStaticModelEntity(std::string filename, std::string_view entityName)
          -> void override;
      auto createAnimatedModelEntity(const AnimatedModelData& modelData) -> void override;
      auto createTerrain() -> void override;
      auto createDefaultCamera() -> void override;
      auto createTestEntity(std::string_view name) -> void override;

    private:
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<Registry> registry;
      std::shared_ptr<sys::CameraSystem> cameraSystem;
      std::shared_ptr<gfx::ResourceManager> resourceManager;
      std::shared_ptr<sys::TransformSystem> transformSystem;
      std::shared_ptr<sys::RenderDataSystem> renderDataSystem;
      std::shared_ptr<sys::AnimationSystem> animationSystem;

      RenderDataTransferHandler transferHandler;
      std::shared_mutex registryMutex{};
      std::unique_ptr<CommandQueue<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&>>
          commandQueue;

      entt::connection entityCreatedConnection;

      cm::gpu::RenderData renderData;

      void entityCreated(entt::registry&, entt::entity) const;
   };
}
