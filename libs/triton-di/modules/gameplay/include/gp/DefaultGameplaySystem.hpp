#pragma once

#include "gp/AssetManager.hpp"
#include "gp/action/IActionSystem.hpp"
#include "gp/components/Transform.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "tr/Events.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"
#include "CommandQueue.hpp"

namespace tr {

class DefaultGameplaySystem : public IGameplaySystem {
public:
  DefaultGameplaySystem(std::shared_ptr<IEventBus> newEventBus,
                        std::shared_ptr<AssetManager> newAssetManager,
                        std::shared_ptr<IActionSystem> newActionSystem);
  ~DefaultGameplaySystem() override;

  DefaultGameplaySystem(const DefaultGameplaySystem&) = delete;
  DefaultGameplaySystem(DefaultGameplaySystem&&) = delete;
  auto operator=(const DefaultGameplaySystem&) -> DefaultGameplaySystem& = delete;
  auto operator=(DefaultGameplaySystem&&) -> DefaultGameplaySystem& = delete;

  void update() override;
  void fixedUpdate() override;

  void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) override;

  auto createStaticModelEntity(std::string filename,
                               std::string_view entityName,
                               std::optional<Transform> initialTransform = std::nullopt)
      -> void override;
  auto createAnimatedModelEntity(const AnimatedModelData& modelData) -> void override;
  auto createTerrain() -> void override;
  auto createDefaultCamera() -> void override;
  auto createTestEntity(std::string_view name) -> void override;
  auto getRegistry() const -> std::shared_ptr<entt::registry> override;

private:
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<AssetManager> assetManager;
  std::shared_ptr<IActionSystem> actionSystem;

  std::shared_ptr<CameraSystem> cameraSystem;
  std::shared_ptr<TransformSystem> transformSystem;
  std::shared_ptr<RenderDataSystem> renderDataSystem;

  RenderDataTransferHandler transferHandler;
  std::shared_ptr<entt::registry> registry;

  mutable TracyLockable(std::shared_mutex, registryMutex);

  std::unique_ptr<CommandQueue<entt::registry&, const std::shared_ptr<AssetManager>&>> commandQueue;

  entt::connection entityCreatedConnection;

  RenderData renderData;

  auto entityCreated([[maybe_unused]] entt::registry& reg,
                     [[maybe_unused]] entt::entity entity) const -> void;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;
  auto handleSwapchainCreated(const SwapchainCreated& event) -> void;
};

}
