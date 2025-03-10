#include "tr/TransvoxelTerrainSystem.hpp"
#include "tr/PlaneGenerator.hpp"

namespace tr {

TransvoxelTerrainSystem::TransvoxelTerrainSystem() {
  densityGenerator = std::make_shared<PlaneGenerator>(glm::vec3(0, 1, 0), 4.f);
}

auto TransvoxelTerrainSystem::registerTerrain(const TerrainCreateInfo& createInfo)
    -> TerrainResult2& {
  Log.debug("registering terrain");
  for (int z = 0; z < createInfo.chunkCount.z; ++z) {
    for (int y = 0; y < createInfo.chunkCount.y; ++y) {
      for (int x = 0; x < createInfo.chunkCount.x; ++x) {
        const auto blockHandle = blockKeygen.getKey();
        const auto location = glm::ivec3(x, y, z);
        const auto name = fmt::format("Chunk ({}, {}, {})", location.x, location.y, location.z);
        blockMap.emplace(blockHandle,
                         BlockResult{.name = name,
                                     .chunkHandle = blockHandle,
                                     .location = location,
                                     .size = createInfo.chunkSize});
      }
    }
  }

  const auto chunks = std::vector<BlockResult>(std::ranges::begin(blockMap | std::views::values),
                                               std::ranges::end(blockMap | std::views::values));

  const auto terrainHandle = terrainKeygen.getKey();

  const auto terrainResult = TerrainResult2{.name = createInfo.name,
                                            .terrainHandle = terrainHandle,
                                            .sdfHandle = 0L, // TODO(matt): delete this
                                            .chunks = chunks};

  terrainMap.emplace(terrainHandle, terrainResult);

  return terrainMap.at(terrainHandle);
};

auto TransvoxelTerrainSystem::triangulateBlock([[maybe_unused]] TerrainHandle terrainHandle,
                                               [[maybe_unused]] BlockHandle blockHandle,
                                               [[maybe_unused]] tr::EntityType blockEntityId,
                                               [[maybe_unused]] glm::ivec3 blockLocation) -> void {
  Log.debug("triangulating block");
}

}
