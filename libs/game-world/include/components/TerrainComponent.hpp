#pragma once

#include "api/vtx/TerrainResult.hpp"

namespace tr {

struct TerrainComponent {
  std::string name;
  TerrainHandle handle;
  std::vector<entt::entity> chunkEntityIds;
};

}
