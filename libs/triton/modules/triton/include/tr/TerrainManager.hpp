#pragma once

#include "SdfGenerator.hpp"
#include "cm/TerrainCreateInfo.hpp"

namespace tr {

class SurfaceExtractor;

class TerrainManager {
public:
  explicit TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator);
  ~TerrainManager();

  TerrainManager(const TerrainManager&) = delete;
  TerrainManager(TerrainManager&&) = delete;
  auto operator=(const TerrainManager&) -> TerrainManager& = delete;
  auto operator=(TerrainManager&&) -> TerrainManager& = delete;

  auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2&;

private:
  std::shared_ptr<SdfGenerator> sdfGenerator;
  std::shared_ptr<SurfaceExtractor> surfaceExtractor;

  MapKey terrainKeygen;
  MapKey chunkKeygen;

  std::unordered_map<TerrainHandle, TerrainResult2> terrainMap;
  std::unordered_map<ChunkHandle, ChunkResult> chunkMap;
};
}
