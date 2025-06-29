#pragma once

#include "api/gw/GameObjectType.hpp"

namespace ed {

struct BlockData {
  tr::GameObjectId entityId;
  glm::ivec3 location;
};

struct TerrainData {
  std::string name;
  glm::vec3 terrainSize;

  std::vector<BlockData> chunkData;
  tr::GameObjectId entityId;

  template <class T>
  void serialize(T& archive) {
    archive(name, terrainSize);
  }
};

}
