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
auto MeshBufferManager::addMesh(const GeometryData& geometryData) -> MeshHandle {

  const auto vertexSize = geometryData.vertices.size() * sizeof(as::Vertex);
  const auto indexSize = geometryData.indices.size() * sizeof(uint32_t);

  const auto newVertexSize = vertexBufferCurrentSize + vertexSize;
  const auto newIndexSize = indexBufferCurrentSize + indexSize;

  const auto newVertexLoadFactor = static_cast<float>(newVertexSize) / vertexBufferMaxSize;
  const auto newIndexLoadFactor = static_cast<float>(newIndexSize) / indexBufferMaxSize;

  if (newVertexLoadFactor > vertexBufferMaxLoad) {
    Log.debug("Vertex Buffer load factor {} exceeded {}, resizing",
              newVertexLoadFactor,
              vertexBufferMaxLoad);
    vertexBufferHandle =
        resourceManager->resizeBuffer(vertexBufferHandle, vertexBufferMaxSize * 1.5f);
    vertexBufferMaxSize *= 1.5f;
  }

  if (newIndexLoadFactor > indexBufferMaxLoad) {
    Log.debug("Index Buffer load factor {} exceeded {}, resizing",
              newIndexLoadFactor,
              indexBufferMaxLoad);
    indexBufferHandle = resourceManager->resizeBuffer(indexBufferHandle, indexBufferMaxSize * 1.5f);
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
