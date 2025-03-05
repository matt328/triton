#include "tr/TerrainManager.hpp"
#include "tr/SdfGenerator.hpp"

namespace tr {

TerrainManager::TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator)
    : sdfGenerator{std::move(newSdfGenerator)} {
}

TerrainManager::~TerrainManager() {
}

/// Returns a reference to the TerrainDefinition for the gameplay system to use since it won't be
/// modified after its created.
auto TerrainManager::registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& {

  const auto chunkHandle = chunkKeygen.getKey();

  const auto location = glm::ivec3(0, 0, 0);
  const auto name = fmt::format("Chunk ({}, {}, {})", location.x, location.y, location.z);
  chunkMap.emplace(chunkHandle,
                   ChunkResult{.name = name, .chunkHandle = chunkHandle, .location = location});

  const auto sdfHandle = sdfGenerator->registerSdf(createInfo.sdfCreateInfo);
  const auto terrainHandle = terrainKeygen.getKey();

  const auto terrainResult = TerrainResult2{.name = createInfo.name,
                                            .terrainHandle = terrainHandle,
                                            .sdfHandle = sdfHandle,
                                            .chunks = {chunkMap.at(chunkHandle)}};

  terrainMap.emplace(terrainHandle, terrainResult);

  return terrainMap.at(terrainHandle);
}

}
