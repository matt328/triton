#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/TransformData.hpp"
#include "fx/GameObjectId.hpp"
#include "fx/ext/StaticObjectDetails.hpp"
#include "fx/ext/DynamicObjectDetails.hpp"
#include "fx/ext/TerrainObjectDetails.hpp"

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

  virtual auto removeEntity(tr::EntityType entityType) -> void = 0;
  virtual auto setTransform(tr::EntityType entityType, TransformData transformData) -> void = 0;

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
