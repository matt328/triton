#include "tr/TerrainManager.hpp"
#include "tr/SdfGenerator.hpp"

namespace tr {

TerrainManager::TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator)
    : sdfGenerator{std::move(newSdfGenerator)} {
}

TerrainManager::~TerrainManager() {
}

auto TerrainManager::registerTerrain(std::string_view name, [[maybe_unused]] glm::vec3 size)
    -> void {
  terrainMap.insert(
      {name.data(),
       {{ChunkDefinition{.size = glm::vec3(9.f, 9.f, 9.f), .location = glm::ivec3(0, 0, 0)}}}});
}

auto TerrainManager::getChunks(std::string_view name) -> std::vector<ChunkDefinition>& {
  return terrainMap.at(name.data());
}

}
