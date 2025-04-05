#pragma once

#include "dd/RenderConfigHandle.hpp"

namespace tr {

struct Renderable {
  RenderConfigHandle renderConfigHandle;
  GeometryHandle geometryHandle;
  MaterialHandle materialHandle;
  ObjectID objectId; // Not actually known until the ObjectDataBuffer is populated for this frame.
};

}
