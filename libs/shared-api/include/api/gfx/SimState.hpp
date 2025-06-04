#pragma once

#include "api/gfx/Geometry.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "bk/Handle.hpp"

namespace tr {

using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

struct StateHandles {
  Handle<Geometry> geometryHandle;
};

struct SimState {
  Timestamp timeStamp{};                     // 8
  uint64_t tag{};                            // 8
  std::vector<GpuObjectData> objectMetadata; // 28
  std::vector<GpuTransformData> positions;   // 20
  std::vector<GpuRotationData> rotations;    // 20
  std::vector<GpuScaleData> scales;          // 20

  // Parallel vector to GpuObjectData.
  std::vector<StateHandles> stateHandles; // 8

  // Pre-allocate memory for the vectors when creating a SimState
  explicit SimState(size_t initialCapacity = 0) {
    objectMetadata.reserve(initialCapacity);
    positions.reserve(initialCapacity);
    rotations.reserve(initialCapacity);
    scales.reserve(initialCapacity);
    stateHandles.reserve(initialCapacity);
  }

  auto ensureCapacity(size_t needed) {
    if (needed > objectMetadata.capacity()) {
      objectMetadata.reserve(needed);
      positions.reserve(needed);
      rotations.reserve(needed);
      scales.reserve(needed);
      stateHandles.reserve(needed);
    }
  }

  auto clear() {
    objectMetadata.clear();
    positions.clear();
    rotations.clear();
    scales.clear();
    stateHandles.clear();
  }
};

}
