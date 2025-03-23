#include "VoxelTerrainContext.hpp"

namespace tr {

VoxelTerrainContext::VoxelTerrainContext(std::shared_ptr<ITerrainSystemProxy> newTerrainSystemProxy)
    : terrainSystemProxy{std::move(newTerrainSystemProxy)} {
}

auto VoxelTerrainContext::getTerrainSystemProxy() -> std::shared_ptr<ITerrainSystemProxy> {
  return terrainSystemProxy;
}

}
