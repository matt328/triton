#include "VoxelTerrainSystemProxy.hpp"

namespace tr {

auto VoxelTerrainSystemProxy::registerTerrain([[maybe_unused]] const TerrainCreateInfo& createInfo)
    -> TerrainResult2& {
  tempResult = TerrainResult2{};
  return tempResult;
}

auto VoxelTerrainSystemProxy::triangulateBlock([[maybe_unused]] TerrainHandle terrainHandle,
                                               [[maybe_unused]] BlockHandle blockHandle,
                                               [[maybe_unused]] GameObjectId blockId,
                                               [[maybe_unused]] glm::ivec3 cellPosition) -> void {
}

}
