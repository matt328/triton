#pragma once

#include "api/gfx/Geometry.hpp"
#include "bk/Handle.hpp"

namespace tr {

struct Renderable {
  std::vector<Handle<Geometry>> geometryHandles;
  std::vector<Handle<TextureTag>> textureHandles;
};

}
