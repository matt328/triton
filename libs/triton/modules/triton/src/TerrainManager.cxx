#include "tr/TerrainManager.hpp"
#include "tr/SdfGenerator.hpp"
#include "cm/GlmToString.hpp"
#include "tr/SurfaceExtractor.hpp"
#include "geo/TerrainGeometryData.hpp"

namespace tr {

TerrainManager::TerrainManager(std::shared_ptr<SdfGenerator> newSdfGenerator,
                               std::shared_ptr<SurfaceExtractor> newSurfaceExtractor,
                               std::shared_ptr<VkResourceManager> newResourceManager,
                               std::shared_ptr<EntityService> newEntityService)
    : sdfGenerator{std::move(newSdfGenerator)},
      surfaceExtractor{std::move(newSurfaceExtractor)},
      resourceManager{std::move(newResourceManager)},
      entityService{std::move(newEntityService)} {
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
                                      tr::EntityType chunkEntityId,
                                      [[maybe_unused]] glm::ivec3 cellLocation) -> void {
  Log.trace("Triangulating Chunk TerrainHandle: {}, ChunkHandle: {}, location: ({},{},{})",
            terrainHandle,
            chunkHandle,
            cellLocation.x,
            cellLocation.y,
            cellLocation.z);

  const auto terrain = terrainMap.at(terrainHandle);
  auto chunk = chunkMap.at(chunkHandle);

  chunk.entityId = chunkEntityId;

  auto vertices = std::vector<as::TerrainVertex>{};
  auto indices = std::vector<uint32_t>{};

  surfaceExtractor->extractSurface(sdfGenerator, terrain.sdfHandle, chunk, vertices, indices);

  Log.trace("Vertices: {}, indices: {}", vertices.size(), indices.size());

  const auto terrainGeometryData = TerrainGeometryData{std::move(vertices), std::move(indices)};

  auto meshHandle = resourceManager->uploadTerrainMesh(terrainGeometryData);

  entityService->addMeshToTerrainChunk(terrain.entityId.value(),
                                       chunk.entityId.value(),
                                       meshHandle);

  // Should this use the standard static mesh geometry buffer, or should the resource manager know
  // how to handle terrain geometry differently?
}

}
