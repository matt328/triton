#include "tr/TerrainManager.hpp"
#include "tr/SdfGenerator.hpp"
#include "cm/GlmToString.hpp"

namespace tr {

TerrainManager::TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator)
    : sdfGenerator{std::move(newSdfGenerator)} {
}

TerrainManager::~TerrainManager() {
}

/// Returns a reference to the TerrainDefinition for the gameplay system to use since it won't be
/// modified after its created.
auto TerrainManager::registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& {

  for (int z = 0; z < createInfo.chunkCount.z; ++z) {
    for (int y = 0; y < createInfo.chunkCount.y; ++y) {
      for (int x = 0; x < createInfo.chunkCount.x; ++x) {
        const auto chunkHandle = chunkKeygen.getKey();
        const auto location = glm::ivec3(x, y, z);
        const auto name = fmt::format("Chunk ({}, {}, {})", location.x, location.y, location.z);
        chunkMap.emplace(chunkHandle,
                         ChunkResult{.name = name,
                                     .chunkHandle = chunkHandle,
                                     .location = location,
                                     .size = createInfo.chunkSize});
      }
    }
  }

  const auto chunks = std::vector<ChunkResult>(std::ranges::begin(chunkMap | std::views::values),
                                               std::ranges::end(chunkMap | std::views::values));

  const auto sdfHandle = sdfGenerator->registerSdf(createInfo.sdfCreateInfo);
  const auto terrainHandle = terrainKeygen.getKey();

  const auto terrainResult = TerrainResult2{.name = createInfo.name,
                                            .terrainHandle = terrainHandle,
                                            .sdfHandle = sdfHandle,
                                            .chunks = chunks};

  terrainMap.emplace(terrainHandle, terrainResult);

  return terrainMap.at(terrainHandle);
}

auto TerrainManager::triangulateChunk([[maybe_unused]] TerrainHandle terrainHandle,
                                      [[maybe_unused]] ChunkHandle chunkHandle,
                                      [[maybe_unused]] glm::ivec3 location) -> void {
  Log.trace("Triangulating Chunk TerrainHandle: {}, ChunkHandle: {}, location: ({},{},{})",
            terrainHandle,
            chunkHandle,
            location.x,
            location.y,
            location.z);
}

}
