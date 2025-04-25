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
    - ObjectDataIndex buffer - enumerates the indices of the objectData that this RenderPass cares
      about.
    - The ObjectData drives everything.
      ObjectData or something needs to be able to index into the DIIC buffer since there will be
      multiple draw calls using the same DIIC buffer, we can't rely on the built in variables like
      drawId.
    - The problem is we start with parallel global buffers, the ObjectBuffer and the DIIC
      buffer, and we have to access the DIIC buffer manually in shaders. without culling, the
      ObjectBuffer and DIIC buffers will be parallel, so whatever indexes into the ObjectBuffer can
      also be used to index into the DIIC buffer. if the GPU culls objects and omits them from the
      DIIC buffer, now the two buffers are not parallel and the compute shader needs to track which
      index in the ObjectBuffer now maps to which index in the DIIC buffer. what if there is just a
      3rd mapping buffer that just maps ObjectData position into DIIC position, and the compute
      shader write this buffer as it writes the DIIC buffer. then the shaders can reference this
      buffer without the cpu having to know about it.
    - I had this a bit backwards. The shaders iterate DrawCommands, not the ObjectBuffer, so we need
      to keep a mapping from DrawCommand's Index to ObjectData's index.
    - Erm Actually
      I don't need to keep a separate buffer. The dIIC method takes a stride parameter so I can tack
      cargo data onto the DIIC buffer's items after the standard vk::DrawIndexedIndirectCommand
  struct. I should place the objectId here, maybe other things as well.

  - The global DIIC buffer will have to be renderPassCount * ObjectCount in size, and a metadata
  buffer uploaded to the GPU at the beginning of each frame will describe at which offset in the
  DIIC buffer each renderpass will find its drawCommands.
  - Don't forget about the CountBuffer and its offsets either.
  - Each ObjectData will have to have a bitmask that tells the compute shader which renderpass'
  slice of the DIIC to write commands.
  - Each RenderPass needs to be supplied with its offset into the Indirect Buffer and the Count
  Buffer on the CPU side.
  - There will be geometry buffers one for each vertex attribute and one for indexes, and then a
  GeometryIndirect buffer that defines the offsets and counts for a single 'geometry'.
  - The ObjectData will contain an index into this GeometryIndirect buffer to be used for vertex
  pulling
  Putting it all together, the renderpass' vertex shaders will iterate over each renderpass' DIIC
  buffer. The buffer will have an object id that will be used to get the GeometryId from the
  ObjectData buffer. The GeometryIndirect entry will be used to find the global locations of the
  index, and the vertex attributes in their respective buffers. Then the other params in the draw
  command will be used to find the local locations of the vertex attributes' values.
  Further, in the fragment shader, the ObjectData will have a MaterialId that will be looked up from
  the global material buffer for texturing.
*/

namespace tr {

/// Defines an object passed from the Game World into the Renderer
struct Renderable {
  RenderStyle renderStyle;
  Handle<GeometryEntry> geometryHandle;
};
}
