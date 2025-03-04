#pragma once

namespace tr {

class SdfGenerator;

struct ChunkDefinition {
  glm::ivec3 size;
  glm::ivec3 location;
};

class TerrainManager {
public:
  explicit TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator);
  ~TerrainManager();

  TerrainManager(const TerrainManager&) = delete;
  TerrainManager(TerrainManager&&) = delete;
  auto operator=(const TerrainManager&) -> TerrainManager& = delete;
  auto operator=(TerrainManager&&) -> TerrainManager& = delete;

  auto registerTerrain(std::string_view name, glm::vec3 size) -> void;

  auto getChunks(std::string_view name) -> std::vector<ChunkDefinition>&;

private:
  std::shared_ptr<SdfGenerator> sdfGenerator;

  std::unordered_map<std::string, std::vector<ChunkDefinition>> terrainMap;
};
}
