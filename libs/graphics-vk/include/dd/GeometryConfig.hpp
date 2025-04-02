#pragma once

#include "api/gfx/Topology.hpp"
#include "dd/VertexList.hpp"

namespace tr {

struct GeometryConfig {
  VertexFormat format;
  Topology topology{};
};

}
