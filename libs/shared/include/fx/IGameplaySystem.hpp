#pragma once

#include "AnimatedModelData.hpp"
#include "cm/TerrainCreateInfo.hpp"
#include "gp/components/Transform.hpp"
#include "gp/EntityService.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "cm/RenderData.hpp"

namespace tr {

using RenderDataTransferHandler = std::function<void(RenderData&)>;

class IGameplaySystem {
public:
  explicit IGameplaySystem() = default;
  virtual ~IGameplaySystem() = default;

  IGameplaySystem(const IGameplaySystem&) = default;
  IGameplaySystem(IGameplaySystem&&) = delete;
  auto operator=(const IGameplaySystem&) -> IGameplaySystem& = default;
  auto operator=(IGameplaySystem&&) -> IGameplaySystem& = delete;

  virtual void update() = 0;
  virtual void fixedUpdate() = 0;

  virtual void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) = 0;

  virtual auto createStaticModelEntity(std::string filename,
                                       std::string_view entityName,
                                       std::optional<Transform> initialTransform)
      -> tr::EntityType = 0;
  virtual auto createAnimatedModelEntity(const AnimatedModelData& modelData,
                                         std::optional<Transform> initialTransform)
      -> tr::EntityType = 0;
  virtual auto createTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& = 0;
  virtual auto createDefaultCamera() -> void = 0;
  virtual auto createTestEntity(std::string_view name) -> void = 0;
  virtual auto removeEntity(tr::EntityType entity) -> void = 0;
  [[nodiscard]] virtual auto getEntityService() const -> std::shared_ptr<EntityService> = 0;

  virtual auto triangulateChunk(tr::EntityType terrainId,
                                tr::EntityType chunkId,
                                glm::ivec3 cellPosition) -> void = 0;
};
}
