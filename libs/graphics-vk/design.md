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

## GPU Data Model/Render Flow

GpuObjectData will drive what needs to be rendered

- Gets recreated each frame on the cpufrom data sent from the GameWorld over the sync point, and then uploaded to the GPU.
- Will be cached so it's only reuploaded to the GPU if something has changed this frame.
- Contains a RenderPass mask describing which RenderPasses should render it
- Contains a model matrix for gpu culling
- Contains an index into the GeometryEntry buffer for generating DIIC entries for each object that passes culling
- Also needs instanceId? to group instances so the GPU knows to add instances to the same DIIC entry.

GeometryBuffers

- Main idea is to store vertex data as non-interleaved arrays.
- System of buffers one for each vertex attribute the renderer knows how to handle.
  - Position
  - Color
  - TextureCoords
  - Normal
  - Tangent
  - JointWeights
  - JointMatrices
  - Indexes
- One main GeometryEntry buffer that describes a single Geometry or Mesh in terms of size and offset(s) into the attribute buffers.
- A field in ObjectData represents the index into the GeometryEntry buffer.

DrawIndexedIndirectCommand (DIIC)

- Object, Geometry, DIIC, Material, all are 'global' one instance of each in the whole renderer.
- Different RenderPass' draw calls will have metadata they need to index into the DIIC and Count buffers, causing the shader programs to be executed based on the DII commands in their slice of the DIIC buffer.
- The DIIC buffer's firstInstance will be used to index into the ObjectBuffer. In the case instanceCount = 1, it will point to the DrawCommand's only associated ObjectData.
- If the instanceCount > 1, the shader programs will be executed instanceCount times, and use gl_InstanceIndex as an additional offset into the ObjectData buffer.
- Compute shader produces a buffer containing these.
- The DIIC buffer is partitioned by RenderPassId, so each renderPass has a predictable slice of this global buffer.
- The size of each slice of the buffer, and the overall size of the buffer will be determined at the beginning of each frame.
- Compute shader will need to carefully construct the DIIC buffer to make sure ObjectData indices always line up, and will need to consider the RenderPassId when setting the firstInstance field, and which position in the DIIC buffer to add the current command's parameters.
- The slices will be sized to fit all of the unique ObjectData (accounting for instances), and any extra space leftover during culling will just be ignored with the use of the count buffer.
- The slice offsets and sizes will also be distributed to each RenderPass each frame so it can use the offsets as params for the drawIndexedIndirectCount() method.

Fragment Shader Ideas

- Pass the materialId as 'flat' so it isn't interpolated.
- Bind the material buffer using its address from the push constants and look up the material properties
and color the fragment appropriately, either by sampling a texture, or using the color attribute of the vertex, depending on what data is present in the Material buffer.

## CPU Render Flow

### Renderable Registration

Renderables will need to be registred with the Renderer. The main goal of this is so their geometry, and optionally their Materials and Textures can be uploaded into the global buffers the renderer uses when rendering.

The game world will just idempotently register geometry, materials, and textures for each object, and recieve handles to each of these in a struct as a return value. The game world will then pass the same struct of handles back over to the renderer, along with their ObjectData to be rendered.

The renderer will have to deduplicate geometry, materials and textures when they're registered, either returning existing handles, or creating new ones and returning those. This will be done asynchronously using a transfer queue so as to not block rendering. If it takes more than one frame to transfer resources, that's fine, since the game world won't ever ask for any of those resources to be rendered until it's gotten their handles back.

### Renderable Eviction

There will be multiple phases of eviction, each having to occur before the next can happen. First, the game world will stop requesting an object be rendered. This may be a temporary condition, a few frames later the gameworld may request those objects be rendered again. The gameworld's logic will make the distinction if an object is safe to be completely evicted. The gameworld will send a message to the renderer notifying it that an object's resources can be evicted. The renderer will internally track shared resource usage, and determine if the resources are not in use by any objects anymore, and asynchronously remove them from arena style buffers by adding to their freelists. Data won't ever actually be zeroed out in gpu buffers, just regions that are no longer being referenced will be marked as being safe to overwrite.

### Rendering

The game world can register objects that it will request be rendered in the future.

- Game World will pass a RenderableData
- RenderableData will contain RenderStyle that a routing table in the Renderer will use to determine which RenderPasses the given RenderableData will need to be involved in.
- RenderStyle should be part of the ObjectData alongside the model matrix so that render styles can be changed each frame if needs be.

#### Renderer Routing Table

```text
Render Pipeline
├── GBuffer Stage
│   ├── GBuffer Pass
│   └── Motion Vectors Pass
├── Lighting Stage
│   └── Deferred Lighting Pass
├── Forward Stage
│   └── Forward Pass (for transparent/foliage/skinned/etc.)
├── Wireframe Stage
│   └── Wireframe Pass
├── Overlay Stage
│   └── Overlay Pass (UI elements, etc)
├── Postprocessing Stage
│   ├── Bloom Pass
│   ├── TAA Pass
│   └── Tonemapping Pass
```

RenderStyles will map to one or more Stages:

RenderStyle::Lit

- GBuffer, Lighting, PostProcessing

RenderStyle::Forward

- Forward, PostProcessing

RenderStyle::Overlay

- Overlay

RenderStyle::Wireframe

- Wireframe

At the beginning of each frame, the requested Renderables will be sorted and assigned into groups according to which Passes will render them. This will be accomplished by adding their ObjectData into the ObjectData buffer in each Pass' slice of the buffer. Metadata about this buffer's slices will be tracked during this process and uploaded into a GPU buffer, as well as distributed to the Passes so they can pass the correct offsets to the drawIndexedIndirectCount calls.

TODO: Class Diagram
