# Data Driven Rendering Design

When creating geometry, in the gameworld, the renderable component will include an opaque handle for a RendererConfig, recieved from the RenderConfigRegistry which will return a compatible RenderConfig, creating them on-demand if a compatible config doesn't exist yet.

Components:

RenderConfigRegistry

- internally stores the RenderConfig in a HashMap and returns an opaque handle
- when loading or generating the geometry in gameworld, the GeometryData will have to contain all the attributes needed to fill out a RenderConfig
- will need an effective operator== or hash functions on RenderConfigs so we can check for a compatible renderconfig before creating a new one every time.

DrawContextFactory

- uses RenderConfigs to create and return DrawContexts on demand, caching them for future use

DrawContext

- stores vertex buffer, index buffer, indirect buffer, and count buffer handles to be retrieved during drawing
- also stores a pipeline handle
- stores handles to GpuBufferEntry buffer, ObjectDataIndex buffer
