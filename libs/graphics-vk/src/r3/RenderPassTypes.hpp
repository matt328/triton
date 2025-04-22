#pragma once

namespace tr {

/*
  1. Flexible VertexAttributes - done
  2. Vertex Pulling, direct AttributeOffsets stored in GeometryRegion
  3. Compile-time shader variant for each AttributeCombination.
    - Need to link a GeometryData to a variant.
      - There will be a RenderPass per variant, so the VertexFormat will determine compatibility,
  but compatibility doens't imply it should be rendered with all compatible renderpasses. Will still
  need an explicit piece of data from the gameworld saying which RenderPass this geometry should be
  rendered with. This should be dynamic per frame data passed from the game-world.
  This will use the renderer's routing table to sort each renderable by renderpass, renderables
  might fall into multiple passes.
  The routing table should also consider MaterialTypes. This should be able to be inferred
  completely based on the Material. Start off with a single MaterialType, but think about how to add
  others. Each material type should be able to have 1 shader variant. A simple check if a textureId
  is present to either sample a texture or use a color attribute is fine.

*/

enum class RenderableType : uint8_t {
  Wireframe = 0,
  Static,
  Dynamic,
  Terrain,
};

}
