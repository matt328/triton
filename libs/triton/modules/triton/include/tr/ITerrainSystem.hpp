#pragma once

#include "cm/TerrainCreateInfo.hpp"
#include "cm/TerrainResult.hpp"

namespace tr {
class ITerrainSystem {
public:
  ITerrainSystem() = default;
  virtual ~ITerrainSystem() = default;

  ITerrainSystem(const ITerrainSystem&) = default;
  ITerrainSystem(ITerrainSystem&&) = delete;
  auto operator=(const ITerrainSystem&) -> ITerrainSystem& = default;
  auto operator=(ITerrainSystem&&) -> ITerrainSystem& = delete;

  virtual auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& = 0;
  virtual auto triangulateBlock(TerrainHandle terrainHandle,
                                BlockHandle blockHandle,
                                tr::EntityType blockEntityId,
                                glm::ivec3 blockLocation) -> void = 0;
};
}
