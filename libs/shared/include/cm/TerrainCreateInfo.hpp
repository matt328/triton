#pragma once

#include "cm/SdfCreateInfo.hpp"

namespace tr {
/// Pass this into GameplaySystem->createTerrain();
struct TerrainCreateInfo {
  std::string name;
  SdfCreateInfo sdfCreateInfo;
  glm::ivec3 chunkCount; // Dimensions of the terrain, in chunks
  glm::ivec3 chunkSize;  // Number of points in the chunk, number of cells is chunkSize.[x|y|z] - 2
};
}
