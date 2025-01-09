#pragma once

#include "cm/Handles.hpp"

namespace tr {
struct GpuMeshData {
  MeshHandle handle{};
  Topology topology{Topology::Triangles};

  friend void swap(GpuMeshData& first, GpuMeshData& second) noexcept {
    using std::swap;
    swap(first.handle, second.handle);
    swap(first.topology, second.topology);
  }
};
}
