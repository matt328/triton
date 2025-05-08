#pragma once

#include "api/gfx/GpuMaterialData.hpp"

namespace tr {

using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

struct SimState {
  Timestamp timeStamp{};
  uint64_t tag{};
  std::vector<GpuObjectData> objectMetadata;
  std::vector<glm::vec3> positions;
  std::vector<glm::quat> rotations;
  std::vector<glm::vec3> scales;

  // Pre-allocate memory for the vectors when creating a SimState
  explicit SimState(size_t numObjects)
      : objectMetadata(numObjects),
        positions(numObjects),
        rotations(numObjects),
        scales(numObjects) {
  }
};

}
