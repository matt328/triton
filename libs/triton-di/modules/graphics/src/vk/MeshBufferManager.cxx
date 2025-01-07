#include "MeshBufferManager.hpp"
#include "as/Vertex.hpp"

#include "vk/VkResourceManager.hpp"

namespace tr {

MeshBufferManager::MeshBufferManager(VkResourceManager* newResourceManager)
    : resourceManager{newResourceManager},
      vertexBufferMaxSize(sizeof(as::Vertex) * 1024),
      indexBufferMaxSize(sizeof(uint32_t) * 1024),
      vertexBufferMaxLoad(0.8f),
      indexBufferMaxLoad(0.8f),
      vertexBufferHandle(
          resourceManager->createGpuVertexBuffer(vertexBufferMaxSize, "Buffer-MeshVertex")),
      indexBufferHandle(
          resourceManager->createGpuIndexBuffer(indexBufferMaxSize, "Buffer-MeshIndex")) {
}

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

  resourceManager->addToMesh(geometryData,
                             vertexBufferHandle,
                             vertexBufferCurrentSize,
                             indexBufferHandle,
                             indexBufferCurrentSize);

  size_t vertexOffset = 0;
  size_t indexOffset = 0;
  for (const auto& bufferEntry : bufferEntries) {
    vertexOffset += bufferEntry.vertexCount;
    indexOffset += bufferEntry.indexCount;
  }

  const auto meshHandle = bufferEntries.size();
  bufferEntries.emplace_back(geometryData.indices.size(),
                             indexOffset,
                             vertexOffset,
                             geometryData.vertices.size());

  vertexBufferCurrentSize = newVertexSize;
  indexBufferCurrentSize = newIndexSize;

  return meshHandle;
}

auto MeshBufferManager::getVertexBufferHandle() const -> BufferHandle {
  return vertexBufferHandle;
}

auto MeshBufferManager::getIndexBufferHandle() const -> BufferHandle {
  return indexBufferHandle;
}

auto MeshBufferManager::removeMesh([[maybe_unused]] MeshHandle meshHandle) -> void {
}

/// RenderDataSystem combines meshHandles and other data from the entities into GpuMeshData
/// and the RenderData contains a list of them.
auto MeshBufferManager::getGpuBufferEntries(const std::vector<GpuMeshData>& meshDataList)
    -> std::vector<GpuBufferEntry> {

  auto instanceDataList = std::vector<GpuBufferEntry>{};

  for (const auto& meshData : meshDataList) {
    const auto& bufferEntry = bufferEntries[meshData.handle];
    instanceDataList.push_back(GpuBufferEntry{.indexCount = bufferEntry.indexCount,
                                              .firstIndex = bufferEntry.firstIndex,
                                              .vertexOffset = bufferEntry.vertexOffset,
                                              .instanceCount = 1,
                                              .firstInstance = 0,
                                              .objectDataId = meshData.objectDataId});
  }

  return instanceDataList;
}

}
