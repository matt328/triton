#pragma once

#include "SdfGenerator.hpp"
#include "cm/TerrainCreateInfo.hpp"
#include "as/TerrainVertex.hpp"
#include "gp/EntityService.hpp"
#include "vk/VkResourceManager.hpp"
#include "ITerrainSystem.hpp"

namespace tr {

class SurfaceExtractor;

class TerrainManager : public ITerrainSystem {
public:
  explicit TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator,
                          std::shared_ptr<SurfaceExtractor> newSurfaceExtractor,
                          std::shared_ptr<VkResourceManager> newResourceManager,
                          std::shared_ptr<EntityService> newEntityService);
  ~TerrainManager();

  TerrainManager(const TerrainManager&) = delete;
  TerrainManager(TerrainManager&&) = delete;
  auto operator=(const TerrainManager&) -> TerrainManager& = delete;
  auto operator=(TerrainManager&&) -> TerrainManager& = delete;

  auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& override;

  auto triangulateBlock(TerrainHandle terrainHandle,
                        BlockHandle chunkHandle,
                        tr::EntityType chunkEntityId,
                        glm::ivec3 location) -> void override;

private:
  std::shared_ptr<SdfGenerator> sdfGenerator;
  std::shared_ptr<SurfaceExtractor> surfaceExtractor;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<EntityService> entityService;

  MapKey terrainKeygen;
  MapKey chunkKeygen;

  std::unordered_map<TerrainHandle, TerrainResult2> terrainMap;
  std::unordered_map<ChunkHandle, BlockResult> chunkMap;
};
}
