# Terrain System

## ECS and Rendering

- TerrainWorld Component
  - contains the functions and values to generate an SDF.
  - determines the bounds of the world.
  - has a current marker in the ctx
- TerrainChunk component
  - has the location of the chunk in the TerrainWorld
  - eventually has a list of modifications

- When a TerrainWorld component is created, it will push it's values into the renderer, where a VoxelGenerator will process the functions and parameters and generate the SDF.
- The renderer will push back a list of TerrainChunks that are currently 'loaded'
- For now, only a few chunks around the camera will be generated and have surfaces extracted so we can render a 3x3 grid of chunks at full resolution.
- As LOD is added, more and more of the chunks will be generated and surface extracted, but at lower levels of detail, and hopefully this can be optimized to render an entire world with an enormous draw distance.

- Most of the work should be implemented in the renderer but still expose parts to gameplay for a few reasons
  1. Eventually pieces of this can be ported into compute shaders, and having the logic inside the renderer will abstract it away from the gameplay system
  2. The gameplay system needs to know about the chunks to enable the realtime modifications to take place without having to expose an api across the gameplay/renderer boundary. Modifications can just happen to the TerrainChunk components, and will be communicated to the renderer via the existing sync point (RenderDataSystem)
  3. The chunks and modifications will need to be known about on the gameplay (CPU) side so that physics and collision detection can have access to that data without streaming it back from the GPU somehow.
