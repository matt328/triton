#pragma once

#include "ITerrainSystem.hpp"
#include "bk/Rando.hpp"

namespace tr {

class SdfGenerator;
class VkResourceManager;
class EntityService;

class DebugTerrainSystem : public ITerrainSystem {
public:
  explicit DebugTerrainSystem(std::shared_ptr<SdfGenerator> newSdfGenerator,
                              std::shared_ptr<VkResourceManager> newResourceManager,
                              std::shared_ptr<EntityService> newEntityService);
  ~DebugTerrainSystem() override = default;

  DebugTerrainSystem(const DebugTerrainSystem&) = delete;
  DebugTerrainSystem(DebugTerrainSystem&&) = delete;
  auto operator=(const DebugTerrainSystem&) -> DebugTerrainSystem& = delete;
  auto operator=(DebugTerrainSystem&&) -> DebugTerrainSystem& = delete;

  /// Registers this instance of a terrain with the TerrainSystem and returns metadata about it so
  /// it can be referenced with subsequent calls.
  auto registerTerrain(const TerrainCreateInfo& createInfo) -> TerrainResult2& override;

  /// Triangulates the given block of the given terrain, giving its entity a renderable component
  /// backed by geometry in a large arena buffer.
  auto triangulateBlock(TerrainHandle terrainHandle,
                        BlockHandle blockHandle,
                        tr::EntityType blockEntityId,
                        glm::ivec3 blockLocation) -> void override;

private:
  std::shared_ptr<SdfGenerator> sdfGenerator;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<EntityService> entityService;

  MapKey terrainKeygen;
  MapKey blockKeygen;

  std::unordered_map<TerrainHandle, TerrainResult2> terrainMap;
  std::unordered_map<BlockHandle, BlockResult> blockMap;
};

}
