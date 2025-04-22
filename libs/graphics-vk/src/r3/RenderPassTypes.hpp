#pragma once

namespace tr {

/*
  RenderPasses will have RenderableTypes they care about rendering.

*/

enum class RenderableType : uint8_t {
  Wireframe = 0,
  Static,
  Dynamic,
  Terrain,
};

}
