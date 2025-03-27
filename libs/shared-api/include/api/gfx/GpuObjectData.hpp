#pragma once

#include "api/gw/Handles.hpp"

namespace tr {

// Maximum size of the GpuObjectData buffer array
constexpr auto MAX_OBJECTS = 128;

struct StaticGpuObjectData {
  glm::mat4 model{};
  TextureHandle textureId{};
  std::array<uint32_t, 3> padding{};
};

struct TerrainGpuObjectData {
  glm::mat4 model{};
  TextureHandle textureId{};
  std::array<uint32_t, 3> padding{};
};

struct DynamicGpuObjectData {
  glm::mat4 model{};
  TextureHandle textureId{};
  uint32_t animationDataIndex{};
  std::array<uint32_t, 2> padding{};
};

}
