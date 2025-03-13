#include "tr/DebugTerrainSystem.hpp"
#include "DebugSurfaceExtractor.hpp"
#include "LinearOctree.hpp"
#include "as/TerrainVertex.hpp"
#include "geo/TerrainGeometryData.hpp"
#include "gp/EntityService.hpp"
#include "tr/SdfGenerator.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

DebugTerrainSystem::DebugTerrainSystem(std::shared_ptr<SdfGenerator> newSdfGenerator,
                                       std::shared_ptr<VkResourceManager> newResourceManager,
                                       std::shared_ptr<EntityService> newEntityService)
    : sdfGenerator{std::move(newSdfGenerator)},
      resourceManager{std::move(newResourceManager)},
      entityService{std::move(newEntityService)} {
}

auto DebugTerrainSystem::registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& {
  Log.debug("registering terrain");

  const auto sdfHandle = sdfGenerator->registerGenerator(createInfo.sdfCreateInfo);

  for (int z = 0; z < createInfo.chunkCount.z; ++z) {
    for (int y = 0; y < createInfo.chunkCount.y; ++y) {
      for (int x = 0; x < createInfo.chunkCount.x; ++x) {
        const auto blockHandle = blockKeygen.getKey();
        const auto location = glm::ivec3(x, y, z);
        const auto name = fmt::format("Chunk ({}, {}, {})", location.x, location.y, location.z);
        blockMap.emplace(blockHandle,
                         BlockResult{.name = name,
                                     .chunkHandle = blockHandle,
                                     .location = location,
                                     .size = createInfo.chunkSize});
      }
    }
  }
  auto ocTree = LinearOctree(glm::ivec3(0, 0, 0), 4096, 8);
  ocTree.debug();

  const auto chunks = std::vector<BlockResult>(std::ranges::begin(blockMap | std::views::values),
                                               std::ranges::end(blockMap | std::views::values));

  const auto terrainHandle = terrainKeygen.getKey();

  const auto terrainResult = TerrainResult2{.name = createInfo.name,
                                            .terrainHandle = terrainHandle,
                                            .sdfHandle = sdfHandle,
                                            .chunks = chunks};

  terrainMap.emplace(terrainHandle, terrainResult);

  return terrainMap.at(terrainHandle);
};

/*
  TODO:
    - Implement triangulateBlock so it returns a LoadedTerrainData which is just a MeshData for now.
      - Eventually this will have texturing information, etc in it.
    - DebugTerrainSystem exposes the triangulate block method so blocks can be triangulated on
      demand
    - Changing getting Sdf values to get the generator and call get() on that.
    - Also a MultipleLodTerrainSystem will be responsible for consulting a LodSystem and
      triangulating its own blocks.

*/
auto DebugTerrainSystem::triangulateBlock([[maybe_unused]] TerrainHandle terrainHandle,
                                          [[maybe_unused]] BlockHandle blockHandle,
                                          [[maybe_unused]] tr::EntityType blockEntityId,
                                          [[maybe_unused]] glm::ivec3 blockLocation) -> void {
  Log.debug("triangulating block");

  const auto surfaceExtractor = std::make_shared<DebugSurfaceExtractor>();

  const auto terrain = terrainMap.at(terrainHandle);
  auto block = blockMap.at(blockHandle);
  block.entityId = blockEntityId;

  auto vertices = std::vector<as::TerrainVertex>{};
  auto indices = std::vector<uint32_t>{};

  const auto generator = sdfGenerator->getGenerator(terrain.sdfHandle);

  surfaceExtractor->extractSurface(generator, block, vertices, indices);

  const auto geometryData = TerrainGeometryData{std::move(vertices), std::move(indices)};
  auto meshHandle = resourceManager->uploadTerrainMesh(geometryData);

  entityService->addMeshToTerrainChunk(terrain.entityId.value(),
                                       block.entityId.value(),
                                       meshHandle);
}

}
