# Renderer Design

## Features

- Open world asset streaming
- Deformable voxel terrain
- Tiled deferred shading for direct lighting
- GI Pass for indirect lighting
- Support multithreaded resource loading and command buffer recording
- Configurable number of frames in flight
- All in bindless indirect rendering
- Vertex pulling

## Things

- BufferManager
- ImageManager
- ShaderModuleFactory
- PipelineBuilder/Factory
- RenderPassFactory
- GpuDataModel
- RenderPass/DrawContext
- FrameGraph
  - execute()
    - initially this won't be much of a graph, just a bunch of renderpasses and the framegraph will know about all of them, and what resources they use and will order them and insert synchronization resources appropriately. If we find this changes frequently we can refactor it to figure this out for itself.

## Processes

Listed from highest level of abstraction to lowest.

- Frame Lifecycle
  - What happens between calling drawFrame() and presenting the frame
    - Renderer Component
      - acquires the next frame from the FrameManager
      - tells the frameGraph to execute() the frame
- Resource Registration/Eviction
  - New renderable is added to the scene
  - Renderable is no longer needed and removed from the scene
- Synchronization
  - Synchronize GPU work submitted
    - When registering renderables
    - Between RenderPasses
- Scene Submission
  - What data is sent across the sync point that the renderer needs to render this frame
- GPU Execution
  - Enumerate and describe every dispatch and draw call
