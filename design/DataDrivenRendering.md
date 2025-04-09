# Data Driven Rendering Design

When creating geometry, in the gameworld, the renderable component will include an opaque handle for a RendererConfig, recieved from the RenderConfigRegistry which will return a compatible RenderConfig, creating them on-demand if a compatible config doesn't exist yet.

## Render Flow

### Registration

- After loading or generating renderables in the gameworld, they need to be registred with the renderer
- The main goal of registration is to assign the renderable to a DrawContext
- Data driven design means DrawContexts will be created on demand as they are needed
- First step of registration is to create a RenderConfig from the RenderableData
  - RenderConfig will be used to index and look up or create a compatible DrawContexts
  - Each DrawContext created will be added to a RenderPass.
  - The engine has hardcoded set of RenderPasses it uses for now, and each RenderPass determines if it will use a given DrawContext based on the RenderConfig

### DrawContexts

DrawContext's main job is to record the commands to the command buffer, using drawIndexedIndirectCount, and should only concern itself with what it immediately needs to set up the buffers to make that call.
RenderPass' will configure the pipeline, and do everything but record the command buffer.
DrawContext needs to contain:

- Shared Handles referenced by DrawContext, assigned by RenderConfig
  - GeometryBuffer
  - Material Buffer
  - Animation Buffer
- LogicalHandles Per-frame instance owned by DrawContext:
  - ObjectData Buffer
  - ObjectDataIndex Buffer
  - BufferEntry Buffer
  - ObjectCount Buffer
  - IndirectDrawCommand Buffer

## Buffers Used by Renderer

```mermaid
graph LR

DrawContext["DrawContext<br/>- Shared Geometry Buffer<br/>- Shared Material Buffer<br/>- Owned BufferEntryBuffer<br/>- Owned ObjectDataBuffer<br/>- Owned ObjectDataIndexBuffer<br/>- Owned ObjectCountBuffer<br/>*Owned buffers are per-frame"]

GeometryBuffer("Geometry Buffer<br/>Contains shared Vertex/Index data`")

GpuBufferEntryBuffer["BufferEntry Buffer<br/>(Offsets into geometry buffer)"]
ObjectDataBuffer["ObjectData Buffer<br/>(Per-object: transform, materialId, meshRef)"]
ObjectDataIndexBuffer["ObjectDataIndex Buffer<br/>(Updated by Compute Shader, indexes into the ObjectDataBuffer)"]
ObjectCountBuffer["ObjectCount Buffer<br/>(Updated by Computer Shader, count of objects to be rendered by this drawIndirect call.)"]
IndirectCommandBuffer["Indirect Draw Buffer<br/>(Updated by Compute DrawIndexedIndirect commands)"]
AnimationDataBuffer["Animation Buffer<br/>(Holds Skeletal animation data per object)"]
MaterialBuffer["Material Buffer<br/>(Holds material instances)"]

%% Edges
GpuBufferEntryBuffer-- Indexes Into ---GeometryBuffer
ObjectDataIndexBuffer --> ObjectDataBuffer
ObjectDataBuffer --> MaterialBuffer
ObjectDataBuffer --> AnimationDataBuffer
