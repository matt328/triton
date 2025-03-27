#pragma once

#include "api/vtx/TerrainCreateInfo.hpp"
#include "api/vtx/TerrainResult.hpp"

/*
  Consider whether its worth the hassle of having terrain in its own lib.
  Otherwise, need to figure out how to isolate separate parts of a lib and keep them as self
  contained modules.
  It could live inside the same library, but use its own injector. Have to think about the folder
  structure for that.
*/

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
