#pragma once

#include "api/gfx/Topology.hpp"
#include "as/VertexList.hpp"

namespace tr {

enum class ShadingMode : uint8_t {
  Textured = 0,
  Colored,
  Wireframe
};

struct RenderConfig {
  VertexFormat vertexFormat;
  Topology topology;
  ShadingMode shadingMode;

  auto operator==(const RenderConfig& other) const -> bool {
    return vertexFormat == other.vertexFormat && topology == other.topology &&
           shadingMode == other.shadingMode;
  }
};

}
