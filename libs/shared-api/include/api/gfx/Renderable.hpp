#pragma once

#include "api/gfx/RenderStyle.hpp"
#include "api/gfx/GeometryEntry.hpp"
#include "bk/Handle.hpp"

/*
  Render data flow
  - GameWorld produces a list of Renderables, passes to Renerer via setRenderData
  - Renderer sorts them into groups based on RenderStyle and the routing table
  - Updates any global buffers
    - ObjectData
    - GeometryCommandData
  - Compute Shader dispatches and updates global buffers
  - Iterates through RenderPasses, calls update() which will update any RenderPass specific buffers
  - Each RenderPass needs to be supplied with its offset into the Indirect Buffer and the Count
  Buffer,
*/

namespace tr {

/// Defines an object passed from the Game World into the Renderer
struct Renderable {
  RenderStyle renderStyle;
  Handle<GeometryEntry> geometryHandle;
};
}
