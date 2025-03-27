#pragma once

#include "api/fx/ITerrainContext.hpp"

namespace tr {

class VoxelTerrainContext : public ITerrainContext {
public:
  explicit VoxelTerrainContext(std::shared_ptr<ITerrainSystemProxy> newTerrainSystemProxy);
  ~VoxelTerrainContext() override = default;

  VoxelTerrainContext(const VoxelTerrainContext&) = default;
  VoxelTerrainContext(VoxelTerrainContext&&) = delete;
  auto operator=(const VoxelTerrainContext&) -> VoxelTerrainContext& = default;
  auto operator=(VoxelTerrainContext&&) -> VoxelTerrainContext& = delete;

  auto getTerrainSystemProxy() -> std::shared_ptr<ITerrainSystemProxy> override;

private:
  std::shared_ptr<ITerrainSystemProxy> terrainSystemProxy;
};

}
