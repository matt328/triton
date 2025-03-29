#pragma once

#include "api/vtx/TerrainCreateInfo.hpp"
#include "api/vtx/TerrainResult.hpp"

namespace tr {

/// Proxy Object to allow direct manipulation of the terrain system by editors and debug engines.
class ITerrainSystemProxy {
public:
  ITerrainSystemProxy() = default;
  virtual ~ITerrainSystemProxy() = default;

  ITerrainSystemProxy(const ITerrainSystemProxy&) = default;
  ITerrainSystemProxy(ITerrainSystemProxy&&) = delete;
  auto operator=(const ITerrainSystemProxy&) -> ITerrainSystemProxy& = default;
  auto operator=(ITerrainSystemProxy&&) -> ITerrainSystemProxy& = delete;

  virtual auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& = 0;

  virtual auto triangulateBlock(TerrainHandle terrainHandle,
                                BlockHandle blockHandle,
                                GameObjectId blockId,
                                glm::ivec3 cellPosition) -> void = 0;
};

}
