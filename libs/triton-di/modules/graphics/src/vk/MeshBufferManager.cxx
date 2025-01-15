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

  auto newVertexLoadFactor =
      static_cast<float>(newVertexSize) / static_cast<float>(vertexBufferMaxSize);
  auto newIndexLoadFactor =
      static_cast<float>(newIndexSize) / static_cast<float>(indexBufferMaxSize);

  {
    size_t calculatedMaxSize = vertexBufferMaxSize;
    while (newVertexLoadFactor > vertexBufferMaxLoad) {
      calculatedMaxSize = static_cast<size_t>(calculatedMaxSize * 1.5f);
      newVertexLoadFactor =
          static_cast<float>(newVertexSize) / static_cast<float>(calculatedMaxSize);
    }

    if (calculatedMaxSize != vertexBufferMaxSize) {
      Log.debug("Resizing vertex buffer to new max size: {}", calculatedMaxSize);
      vertexBufferHandle = resourceManager->resizeBuffer(vertexBufferHandle, calculatedMaxSize);
      vertexBufferMaxSize = calculatedMaxSize;
    }
  }

  {
    size_t calculatedIndexMaxSize = indexBufferMaxSize;
    while (newIndexLoadFactor > indexBufferMaxLoad) {
      calculatedIndexMaxSize = static_cast<size_t>(calculatedIndexMaxSize * 1.5f);
      newIndexLoadFactor =
          static_cast<float>(newIndexSize) / static_cast<float>(calculatedIndexMaxSize);
    }

    if (calculatedIndexMaxSize != indexBufferMaxSize) {
      Log.debug("Resizing index buffer to new max size: {}", calculatedIndexMaxSize);
      indexBufferHandle = resourceManager->resizeBuffer(indexBufferHandle, calculatedIndexMaxSize);
      indexBufferMaxSize = calculatedIndexMaxSize;
    }
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

/// RenderDataSystem combines meshHandles and other data from the entities into RenderMeshData
/// and the RenderData contains a list of them.
auto MeshBufferManager::getGpuBufferEntries(const std::vector<RenderMeshData>& meshDataList)
    -> std::vector<GpuBufferEntry>& {
  ZoneNamedN(var, "mbm getGpuBufferEntries", true);
  gpuBufferEntryList.clear();

  for (const auto& meshData : meshDataList) {
    ZoneNamedN(var, "processMeshData", true);
    const auto& bufferEntry = bufferEntries[meshData.handle];
    gpuBufferEntryList.push_back(GpuBufferEntry{.indexCount = bufferEntry.indexCount,
                                                .firstIndex = bufferEntry.firstIndex,
                                                .vertexOffset = bufferEntry.vertexOffset,
                                                .instanceCount = 1,
                                                .firstInstance = 0,
                                                .padding = 0});
  }

  return gpuBufferEntryList;
}

}
