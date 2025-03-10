#pragma once

#include "ITerrainSystem.hpp"
#include "cm/Rando.hpp"

namespace tr {

class IDensityGenerator;

class TransvoxelTerrainSystem : public ITerrainSystem {
public:
  TransvoxelTerrainSystem();
  ~TransvoxelTerrainSystem() override = default;

  TransvoxelTerrainSystem(const TransvoxelTerrainSystem&) = delete;
  TransvoxelTerrainSystem(TransvoxelTerrainSystem&&) = delete;
  auto operator=(const TransvoxelTerrainSystem&) -> TransvoxelTerrainSystem& = delete;
  auto operator=(TransvoxelTerrainSystem&&) -> TransvoxelTerrainSystem& = delete;

  auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& override;
  auto triangulateBlock(TerrainHandle terrainHandle,
                        BlockHandle blockHandle,
                        tr::EntityType blockEntityId,
                        glm::ivec3 blockLocation) -> void override;

private:
  std::shared_ptr<IDensityGenerator> densityGenerator;

  MapKey terrainKeygen;
  MapKey blockKeygen;

  std::unordered_map<TerrainHandle, TerrainResult2> terrainMap;
  std::unordered_map<BlockHandle, BlockResult> blockMap;
};

}
