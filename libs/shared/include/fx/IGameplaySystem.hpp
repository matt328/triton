#pragma once

#include "cm/AnimatedModelData.hpp"
#include "cm/TerrainCreateInfo.hpp"
#include "cm/TransformData.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "cm/RenderData.hpp"
#include "cm/TerrainResult.hpp"

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
                                       std::optional<TransformData> initialTransform)
      -> tr::EntityType = 0;
  virtual auto createAnimatedModelEntity(const AnimatedModelData& modelData,
                                         std::optional<TransformData> initialTransform)
      -> tr::EntityType = 0;
  virtual auto createTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& = 0;
  virtual auto createDefaultCamera() -> void = 0;
  virtual auto createTestEntity(std::string_view name) -> void = 0;
  virtual auto removeEntity(tr::EntityType entity) -> void = 0;

  virtual auto triangulateChunk(tr::EntityType terrainId,
                                tr::EntityType chunkId,
                                glm::ivec3 cellPosition) -> void = 0;
};
}
