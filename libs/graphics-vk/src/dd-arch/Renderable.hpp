#pragma once

#include "dd/RenderConfigHandle.hpp"

namespace tr {

using GeometryHandle = size_t;
using MaterialHandle = size_t;
using ObjectID = size_t;

struct Renderable {
  RenderConfigHandle renderConfigHandle;
  GeometryHandle geometryHandle;
  MaterialHandle materialHandle;
  ObjectID objectId; // Not actually known until the ObjectDataBuffer is populated for this frame.
};

}
