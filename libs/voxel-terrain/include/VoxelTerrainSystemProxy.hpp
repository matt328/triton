#pragma once

#include "api/ext/ITerrainSystemProxy.hpp"

#include "api/gw/GameObjectType.hpp"

namespace tr {

class VoxelTerrainSystemProxy : public ITerrainSystemProxy {
public:
  VoxelTerrainSystemProxy();
  ~VoxelTerrainSystemProxy() override;

  VoxelTerrainSystemProxy(const VoxelTerrainSystemProxy&) = delete;
  VoxelTerrainSystemProxy(VoxelTerrainSystemProxy&&) = delete;
  auto operator=(const VoxelTerrainSystemProxy&) -> VoxelTerrainSystemProxy& = delete;
  auto operator=(VoxelTerrainSystemProxy&&) -> VoxelTerrainSystemProxy& = delete;

  auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& override;

  auto triangulateBlock(TerrainHandle terrainHandle,
                        BlockHandle blockHandle,
                        GameObjectId blockId,
                        glm::ivec3 cellPosition) -> void override;

private:
  TerrainResult2 terrainResult{};
};

}
