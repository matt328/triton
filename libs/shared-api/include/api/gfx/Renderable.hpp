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
      - sets modelMatrix, geometryEntryId, optional materialId, and animationIds
      - geometryEntryId is an index into the GeometryEntryData array that will have been uploaded
        asynchronously
      - geometryEntryId will be contained in the value returned from asynchronously uploading a
        Renderable, and stored in the gameobject in the gameworld, so that it can be passed back to
        the renderer.

  - Compute Shader dispatches and updates global buffers
    - Updates
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
