#pragma once

#include "cm/Handles.hpp"

namespace tr {

/// Transfer Object between world and render. Used to identify regions of Vertex and Index Buffer
/// that need to be rendered.
struct RenderMeshData {
  MeshHandle handle{};
  Topology topology{Topology::Triangles};

  /// Avoid copies in move operations
  friend void swap(RenderMeshData& first, RenderMeshData& second) noexcept {
    using std::swap;
    swap(first.handle, second.handle);
    swap(first.topology, second.topology);
  }
};

}
