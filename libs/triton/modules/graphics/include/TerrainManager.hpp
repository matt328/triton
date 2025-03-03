#pragma once

namespace tr {

struct ChunkDefinition {
  glm::ivec3 size;
  glm::ivec3 location;
};

class TerrainManager {
public:
  TerrainManager();
  ~TerrainManager();

  TerrainManager(const TerrainManager&) = delete;
  TerrainManager(TerrainManager&&) = delete;
  auto operator=(const TerrainManager&) -> TerrainManager& = delete;
  auto operator=(TerrainManager&&) -> TerrainManager& = delete;

  auto registerTerrain(std::string_view name, glm::vec3 size) -> void;

  auto getChunks(std::string_view name) -> std::vector<ChunkDefinition>&;
};
}
