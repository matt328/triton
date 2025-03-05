#pragma once

#include "cm/TerrainResult.hpp"

namespace tr {

struct TerrainComponent {
  std::string name;
  TerrainHandle handle;
  std::vector<tr::EntityType> chunkEntityIds;
};

}
