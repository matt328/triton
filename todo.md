# TODO

- Remove entities and renderables
- Fire up animated model again
- create some debug rendering utils
  - wireframe cube
  - vertex labels
- Revisit transvoxel algorithm for terrain
- figure out lighting
- character walk around terrain
- skybox/sphere

## Multiple Vertex Format Support

- AssetManager now has SkinnedGeometryData and StaticGeometryData which it differentiates by whether
  the model file has staticVertices or skinnedVertices.
- It has separate maps for storing the vertex data before its uploaded to the gpu.

- VkResourceManager and MeshBufferManager are both kinda intertwined. They would each need separate
  code paths to handle each ModelData of each format.
- VkResourceManager would need separate MeshBufferManagers to manage separate mesh buffers, one for
  each vertex format.

- First, untangle VkResourceManager and MeshBufferManager's relationship. Maybe just move some
  functionality from VkResourceManager into MeshBufferManager so that MeshBufferManager doesn't
  have to call back into VkResourceManager, and the VkResourceManager can just own several
  MeshBufferManagers

- Look into making MeshBufferManager templated on the Vertex type. See if adding skinning data would
  affect any functionality in this class, it might not since it's just managing the buffer, and
  doesn't really care what's in it.

- Consider another component over resource manager that can own resource manager and multiple
  MeshBufferManagers, and keep low level functionality in ResourceManager and composed functionality
  in MeshBufferManagers
