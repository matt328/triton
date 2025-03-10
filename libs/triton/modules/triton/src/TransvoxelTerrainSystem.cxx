#include "tr/TransvoxelTerrainSystem.hpp"
#include "tr/PlaneGenerator.hpp"

namespace tr {

TransvoxelTerrainSystem::TransvoxelTerrainSystem() {
  densityGenerator = std::make_shared<PlaneGenerator>();
}

auto TransvoxelTerrainSystem::registerTerrain(const TerrainCreateInfo& createInfo)
    -> TerrainResult2& {
  Log.debug("registering terrain");
};

auto TransvoxelTerrainSystem::triangulateBlock(TerrainHandle terrainHandle,
                                               BlockHandle blockHandle,
                                               tr::EntityType blockEntityId,
                                               glm::ivec3 blockLocation) -> void {
  Log.debug("triangulating block");
}

}
