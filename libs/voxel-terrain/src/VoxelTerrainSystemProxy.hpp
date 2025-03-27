#pragma once

#include "api/ext/ITerrainSystemProxy.hpp"

#include "api/gw/GameObjectType.hpp"

namespace tr {

class VoxelTerrainSystemProxy : public ITerrainSystemProxy {
public:
  VoxelTerrainSystemProxy() = default;
  ~VoxelTerrainSystemProxy() override = default;

  VoxelTerrainSystemProxy(const VoxelTerrainSystemProxy&) = default;
  VoxelTerrainSystemProxy(VoxelTerrainSystemProxy&&) = delete;
  auto operator=(const VoxelTerrainSystemProxy&) -> VoxelTerrainSystemProxy& = default;
  auto operator=(VoxelTerrainSystemProxy&&) -> VoxelTerrainSystemProxy& = delete;

  auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& override;

  auto triangulateBlock(TerrainHandle terrainHandle,
                        BlockHandle blockHandle,
                        tr::GameObjectId blockId,
                        glm::ivec3 cellPosition) -> void override;

private:
  TerrainResult2 tempResult;
};

}
