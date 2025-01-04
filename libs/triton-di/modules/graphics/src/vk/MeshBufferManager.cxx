#include "MeshBufferManager.hpp"
#include "as/Vertex.hpp"

namespace tr {

MeshBufferManager::MeshBufferManager(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)},
      vertexBufferMaxSize(sizeof(as::Vertex) * 1024),
      indexBufferMaxSize(sizeof(uint32_t) * 1024),
      vertexBufferMaxLoad(0.8f),
      indexBufferMaxLoad(0.8f),
      vertexBufferHandle(
          resourceManager->createGpuVertexBuffer(vertexBufferMaxSize, "Buffer-MeshVertex")),
      indexBufferHandle(
          resourceManager->createGpuIndexBuffer(indexBufferMaxSize, "Buffer-MeshIndex")) {
}

// A meshhandle will have to be an index into something tracked by the MeshBufferManager
// I think they'll just be added to an entity and when the renderdatasystem is called it'll just
// hand the meshhandle back to the meshbuffermanager to produce a list of Object/Instance Data that
// will be passed to the compute shader to generate the draw commands
/*
  Also this meshbuffermanager will need to provide it's 'current' index/vertex buffer to the
  IndirectRenderTask somehow.
  For synchronization so that adding a mesh doesn't stall the renderer, we'll need to double buffer
  the vertex/index buffers while a copy operation is made. Because of this, the MeshBufferManager
  will need to provide the vertex and index buffer handles to the IndiretRenderTask each frame, and
  we can ensure on the CPU side that it always presents a consistent view of the buffers and their
  Object/Instance data. Basically addMesh will always be called from a background thread, so it can
  just wait on the copy operation and swap the buffers and update the Object/Instance data when it's
  done.

  First we need to figure out where this class lives and fits into the game world -> render world
  communication. There should be some facade over this and the VkResourceManager that the game world
  can interface with. So the game world won't call addMesh directly, it'll call something like
  addStaticModelInstance and that will know to call addMesh to this meshBufferManager.

  Once that flow is working and we can add and remove static meshes from the UI and have the buffer
  grow and shrink, look at adapting this to support instancing.

  Also then we'd need an instance of this that handles animated meshes which would be pretty similar
  but with the indexed animation data bound to a separate resizable buffer which each object's
  instance/object data would index into

  After static and dynamic meshes are working, pick up the marching cubes terrain stuff.
  All rendering except ImGui should be using bindless draw indirect with buffer device addressing.
  Heck descriptor sets they are confusing af.
*/
auto MeshBufferManager::addMesh(const GeometryData& geometryData) -> MeshHandle {

  const auto vertexSize = geometryData.vertices.size() * sizeof(as::Vertex);
  const auto indexSize = geometryData.indices.size() * sizeof(uint32_t);

  const auto newVertexSize = vertexBufferCurrentSize + vertexSize;
  const auto newIndexSize = indexBufferCurrentSize + indexSize;

  const auto newVertexLoadFactor =
      static_cast<float>(newVertexSize) / static_cast<float>(vertexBufferMaxSize);
  const auto newIndexLoadFactor =
      static_cast<float>(newIndexSize) / static_cast<float>(indexBufferMaxSize);

  if (newVertexLoadFactor > vertexBufferMaxLoad) {
    Log.debug("Vertex Buffer load factor {} exceeded {}, resizing",
              newVertexLoadFactor,
              vertexBufferMaxLoad);
    vertexBufferHandle =
        resourceManager->resizeBuffer(vertexBufferHandle,
                                      static_cast<size_t>(vertexBufferMaxSize * 1.5f));
    vertexBufferMaxSize *= 1.5f;
  }

  if (newIndexLoadFactor > indexBufferMaxLoad) {
    Log.debug("Index Buffer load factor {} exceeded {}, resizing",
              newIndexLoadFactor,
              indexBufferMaxLoad);
    indexBufferHandle =
        resourceManager->resizeBuffer(indexBufferHandle,
                                      static_cast<size_t>(indexBufferMaxSize * 1.5f));
    indexBufferMaxSize *= 1.5f;
  }

  resourceManager->addToMesh(geometryData, vertexBufferHandle, indexBufferHandle);

  size_t vertexOffset = 0;
  size_t indexOffset = 0;
  for (const auto& bufferEntry : bufferEntries) {
    vertexOffset += bufferEntry.vertexCount;
    indexOffset += bufferEntry.indexCount;
  }

  const auto meshHandle = bufferEntries.size();
  bufferEntries.emplace_back(vertexOffset,
                             geometryData.vertices.size(),
                             indexOffset,
                             geometryData.indices.size());

  vertexBufferCurrentSize = newVertexSize;
  indexBufferCurrentSize = newIndexSize;

  return meshHandle;
}

auto MeshBufferManager::removeMesh([[maybe_unused]] MeshHandle meshHandle) -> void {
}

auto MeshBufferManager::getInstanceData([[maybe_unused]] std::vector<GpuMeshData> meshHandles)
    -> std::vector<InstanceData2> {
  return {};
}

}
