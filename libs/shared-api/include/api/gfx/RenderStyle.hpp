#pragma once

namespace tr {
/// Game World attribute describing how Geometry should be rendered.
enum class RenderStyle : uint8_t {
  Wireframe = 0,
  Lit,
  Shadow,
  Reflection,
  Terrain
};
}
