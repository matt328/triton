#pragma once

#include "api/gfx/GpuMaterialData.hpp"

namespace tr {

using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

struct SimState {
  Timestamp timeStamp{};
  uint64_t tag{};
  std::vector<GpuObjectData> objectMetadata;
  std::vector<GpuTransformData> positions;
  std::vector<GpuRotationData> rotations;
  std::vector<GpuScaleData> scales;

  // Pre-allocate memory for the vectors when creating a SimState
  explicit SimState(size_t numObjects)
      : objectMetadata(numObjects),
        positions(numObjects),
        rotations(numObjects),
        scales(numObjects) {
  }
};

}
