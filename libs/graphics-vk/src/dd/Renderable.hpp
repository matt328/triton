#pragma once

namespace tr {

using RenderConfigHandle = uint64_t;

struct Renderable {
  RenderConfigHandle renderConfigHandle;
  GeometryHandle geometryHandle;
  MaterialHandle materialHandle;
  ObjectID objectId;
};

}
