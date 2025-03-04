#pragma once

#include "TerrainManager.hpp"
#include "gp/AssetManager.hpp"
#include "gp/action/IActionSystem.hpp"
#include "gp/components/Transform.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "tr/Events.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"

namespace tr {

class DefaultGameplaySystem : public IGameplaySystem {
public:
  DefaultGameplaySystem(std::shared_ptr<IEventBus> newEventBus,
                        std::shared_ptr<AssetManager> newAssetManager,
                        std::shared_ptr<IActionSystem> newActionSystem,
                        std::shared_ptr<CameraSystem> newCameraSystem,
                        std::shared_ptr<TransformSystem> newTransformSystem,
                        std::shared_ptr<AnimationSystem> newAnimationSystem,
                        std::shared_ptr<RenderDataSystem> newRenderDataSystem,
                        std::shared_ptr<EntityService> entityService,
                        std::shared_ptr<TerrainManager> newTerrainManager);
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
                               std::optional<Transform> initialTransform)
      -> tr::EntityType override;
  auto createAnimatedModelEntity(const AnimatedModelData& modelData,
                                 std::optional<Transform> initialTransform)
      -> tr::EntityType override;
  auto createTerrain(std::string_view name, glm::vec3 terrainSize) -> TerrainResult override;
  auto createDefaultCamera() -> void override;
  auto createTestEntity(std::string_view name) -> void override;
  auto removeEntity(tr::EntityType entity) -> void override;
  [[nodiscard]] auto getEntityService() const -> std::shared_ptr<EntityService> override;

private:
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<AssetManager> assetManager;
  std::shared_ptr<IActionSystem> actionSystem;
  std::shared_ptr<CameraSystem> cameraSystem;
  std::shared_ptr<TransformSystem> transformSystem;
  std::shared_ptr<AnimationSystem> animationSystem;
  std::shared_ptr<RenderDataSystem> renderDataSystem;
  std::shared_ptr<EntityService> entityService;
  std::shared_ptr<TerrainManager> terrainManager;

  RenderDataTransferHandler transferHandler;

  entt::connection entityCreatedConnection;

  RenderData renderData;

  auto entityCreated([[maybe_unused]] entt::registry& reg,
                     [[maybe_unused]] entt::entity entity) const -> void;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;
  auto handleSwapchainCreated(const SwapchainCreated& event) -> void;
};

}
