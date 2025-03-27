#pragma once

#include "api/vtx/TerrainResult.hpp"
namespace tr {

struct ChunkComponent {
  glm::ivec3 location;
  glm::ivec3 dimensions;
  ChunkHandle handle;
};

}
