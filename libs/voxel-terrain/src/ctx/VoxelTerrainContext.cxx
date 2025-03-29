#include "VoxelTerrainContext.hpp"

namespace tr {

VoxelTerrainContext::VoxelTerrainContext(std::shared_ptr<ITerrainSystemProxy> newTerrainSystemProxy)
    : terrainSystemProxy{std::move(newTerrainSystemProxy)} {
}

auto VoxelTerrainContext::getTerrainSystemProxy() -> std::shared_ptr<ITerrainSystemProxy> {
  return terrainSystemProxy;
}

auto VoxelTerrainContext::registerGameObjectProxy(
    [[maybe_unused]] std::shared_ptr<IGameObjectProxy> newGameObjectProxy) -> void {
  Log.trace("VoxelTerrainContext Registering gameObjectProxy");
}

auto VoxelTerrainContext::registerResourceProxy(
    [[maybe_unused]] std::shared_ptr<IResourceProxy> newResourceProxy) -> void {
  Log.trace("VoxelTerrainContext Registering resourceProxy");
}

}
