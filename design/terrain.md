# Terrain System

## ECS and Rendering

- In general implement transvoxel algorithm.
- Each block will be an entity in the ECS
- Use an octree to determine lod for each block
- utilize multithreading for calculating block updates, and applying block updates
- Ensure these are encapsulated into 'pure' tasks with defined inputs and outputs so they can be multithreaded easily
- it looks like each block will have the same number of vertices, higher lods will just be smaller and more detailed
- have a single large vertex/index buffer pair using arena buffer, but will need to design an efficient way to
  update regions of the arena buffer without delaying rendering. Possibly double buffer a limited number of regions
  so they can be updated without locking
- Only allow so many regions to be double buffered at a time to save on memory, and queue up updated blocks to be
  applied over a few frames.
- Profile and log metrics to determine how many 'extra' block regions will be needed
