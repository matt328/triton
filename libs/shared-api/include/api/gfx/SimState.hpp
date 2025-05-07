#pragma once

#include "api/gfx/GpuMaterialData.hpp"

namespace tr {
struct SimState {
  std::vector<GpuObjectData> objectMetadata;
  std::vector<glm::vec3> positions;
  std::vector<glm::quat> rotations;
  std::vector<glm::vec3> scales;
};
}
