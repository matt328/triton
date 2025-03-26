#pragma once

#include "gw/AnimatedModelData.hpp"
#include "cm/TerrainCreateInfo.hpp"
#include "cm/TransformData.hpp"
#include "gw/GameObjectType.hpp"
#include "cm/RenderData.hpp"
#include "cm/TerrainResult.hpp"

namespace tr {

using RenderDataTransferHandler = std::function<void(RenderData&)>;

class IGameWorldSystem {
public:
  explicit IGameWorldSystem() = default;
  virtual ~IGameWorldSystem() = default;

  IGameWorldSystem(const IGameWorldSystem&) = default;
  IGameWorldSystem(IGameWorldSystem&&) = delete;
  auto operator=(const IGameWorldSystem&) -> IGameWorldSystem& = default;
  auto operator=(IGameWorldSystem&&) -> IGameWorldSystem& = delete;

  virtual void update() = 0;
  virtual void fixedUpdate() = 0;

  virtual void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) = 0;

  virtual auto createStaticModelEntity(std::string filename,
                                       std::string_view entityName,
                                       std::optional<TransformData> initialTransform)
      -> tr::GameObjectId = 0;
  virtual auto createAnimatedModelEntity(const AnimatedModelData& modelData,
                                         std::optional<TransformData> initialTransform)
      -> tr::GameObjectId = 0;
  virtual auto createTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& = 0;
  virtual auto createDefaultCamera() -> void = 0;
  virtual auto removeEntity(tr::GameObjectId entity) -> void = 0;

  virtual auto triangulateChunk(tr::GameObjectId terrainId,
                                tr::GameObjectId chunkId,
                                glm::ivec3 cellPosition) -> void = 0;
};
}
