#pragma once

#include "cm/Handles.hpp"
#include "cm/AssetStructs.hpp"

namespace tr {

/// Transfer Object between world and render. Used to identify regions of Vertex and Index Buffer
/// that need to be rendered.
struct RenderMeshData {
  MeshHandle handle{};
  Topology topology{Topology::Triangles};
  std::optional<TextureHandle> textureHandle = std::nullopt;

  /// Avoid copies in move operations
  friend void swap(RenderMeshData& first, RenderMeshData& second) noexcept {
    using std::swap;
    swap(first.handle, second.handle);
    swap(first.topology, second.topology);
    swap(first.textureHandle, second.textureHandle);
  }
};

}
