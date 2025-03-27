#pragma once

#include "api/gw/GameObjectType.hpp"
#include "api/gw/TransformData.hpp"
#include "api/gw/GameObjectType.hpp"
#include "api/ext/StaticObjectDetails.hpp"
#include "api/ext/DynamicObjectDetails.hpp"
#include "api/ext/TerrainObjectDetails.hpp"

namespace tr {
/// GameObject proxy class specifically for editors and possibly debug engines to directly
/// manipulate the game world. Might not even exist in an actual game.
class IGameObjectProxy {
public:
  IGameObjectProxy() = default;
  virtual ~IGameObjectProxy() = default;

  IGameObjectProxy(const IGameObjectProxy&) = default;
  IGameObjectProxy(IGameObjectProxy&&) = delete;
  auto operator=(const IGameObjectProxy&) -> IGameObjectProxy& = default;
  auto operator=(IGameObjectProxy&&) -> IGameObjectProxy& = delete;

  virtual auto removeEntity(tr::GameObjectId entityType) -> void = 0;
  virtual auto setTransform(tr::GameObjectId entityType, TransformData transformData) -> void = 0;

  virtual auto getEditableGameObjects() -> std::vector<GameObjectId> = 0;
  virtual auto getStaticObjectDetails() -> StaticObjectDetails = 0;
  virtual auto getDynamicObjectDetails() -> DynamicObjectDetails = 0;
  virtual auto getTerrainObjectDetails() -> TerrainObjectDetails = 0;

  virtual auto setStaticObjectDetails(GameObjectId id,
                                      const StaticObjectDetails& details) -> void = 0;
  virtual auto setDynamicObjectDetails(GameObjectId id,
                                       const DynamicObjectDetails& details) -> void = 0;
  virtual auto setTerrainObjectDetails(GameObjectId id,
                                       const TerrainObjectDetails& details) -> void = 0;

  // Debug Methods
  virtual auto triangulateChunk(GameObjectId terrainId,
                                GameObjectId chunkId,
                                glm::ivec3 cellPosition) -> void = 0;
};

}
