#pragma once

#include "cm/TerrainCreateInfo.hpp"
#include "cm/TerrainResult.hpp"

namespace tr {

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
                                tr::EntityType blockId,
                                glm::ivec3 cellPosition) -> void = 0;
};

}
