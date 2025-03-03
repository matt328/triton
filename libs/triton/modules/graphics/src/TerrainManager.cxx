#include "TerrainManager.hpp"

namespace tr {

TerrainManager::TerrainManager() {
}

TerrainManager::~TerrainManager() {
}

auto TerrainManager::registerTerrain(std::string_view name, glm::vec3 size) -> void {
}

auto TerrainManager::getChunks(std::string_view name) -> std::vector<ChunkDefinition>& {
  return {};
}

}
