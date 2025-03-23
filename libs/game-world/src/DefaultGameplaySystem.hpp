#pragma once

#include "fx/IGameWorldSystem.hpp"
#include "fx/Events.hpp"

namespace tr {

class IAssetService;
class IActionSystem;
class AnimationSystem;
class CameraSystem;
class TransformSystem;
class RenderDataSystem;
class ITerrainSystemProxy;
class IEventBus;
class EntityService;
class IResourceProxy;

class DefaultGameplaySystem : public IGameWorldSystem {
public:
  DefaultGameplaySystem(std::shared_ptr<IEventBus> newEventBus,
                        std::shared_ptr<IAssetService> newAssetService,
                        std::shared_ptr<IActionSystem> newActionSystem,
                        std::shared_ptr<CameraSystem> newCameraSystem,
                        std::shared_ptr<TransformSystem> newTransformSystem,
                        std::shared_ptr<AnimationSystem> newAnimationSystem,
                        std::shared_ptr<RenderDataSystem> newRenderDataSystem,
                        std::shared_ptr<EntityService> entityService,
                        std::shared_ptr<ITerrainSystemProxy> newTerrainSystemProxy,
                        std::shared_ptr<IResourceProxy> newResourceProxy);
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
                               std::optional<TransformData> initialTransform)
      -> tr::EntityType override;

  auto createAnimatedModelEntity(const AnimatedModelData& modelData,
                                 std::optional<TransformData> initialTransform)
      -> tr::EntityType override;

  auto createTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& override;

  auto createDefaultCamera() -> void override;

  auto removeEntity(tr::EntityType entity) -> void override;

  auto triangulateChunk(tr::EntityType terrainId,
                        tr::EntityType chunkId,
                        glm::ivec3 cellPosition) -> void override;

private:
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<IActionSystem> actionSystem;
  std::shared_ptr<CameraSystem> cameraSystem;
  std::shared_ptr<TransformSystem> transformSystem;
  std::shared_ptr<AnimationSystem> animationSystem;
  std::shared_ptr<RenderDataSystem> renderDataSystem;
  std::shared_ptr<EntityService> entityService;
  std::shared_ptr<ITerrainSystemProxy> terrainSystemProxy;
  std::shared_ptr<IResourceProxy> resourceProxy;

  RenderDataTransferHandler transferHandler;

  entt::connection entityCreatedConnection;

  RenderData renderData;

  auto entityCreated([[maybe_unused]] entt::registry& reg,
                     [[maybe_unused]] entt::entity entity) const -> void;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;
  auto handleSwapchainCreated(const SwapchainCreated& event) -> void;
};

}
