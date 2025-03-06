#pragma once

#include "cm/EntitySystemTypes.hpp"

namespace tr {
using SdfHandle = uint64_t;
using ChunkHandle = uint64_t;
using TerrainHandle = uint64_t;

struct ChunkResult {
  std::string name;
  ChunkHandle chunkHandle;
  glm::ivec3 location;
  glm::ivec3 size;
  std::optional<tr::EntityType> entityId = std::nullopt;
};

/// Returned from TerrainManager::createTerrain() without entityIds
/// Passed into EntityService::terrainCreated() by non-const ref and ES augments it with EntityIds
struct TerrainResult2 {
  std::string name;
  TerrainHandle terrainHandle;
  SdfHandle sdfHandle;
  std::vector<ChunkResult> chunks;
  std::optional<tr::EntityType> entityId = std::nullopt;
};
}
