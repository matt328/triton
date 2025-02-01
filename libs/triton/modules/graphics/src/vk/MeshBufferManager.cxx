#include "MeshBufferManager.hpp"

#include "vk/BufferManager.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

MeshBufferManager::MeshBufferManager(std::shared_ptr<BufferManager> newBufferManager,
                                     size_t vertexSize,
                                     std::string_view bufferName)
    : bufferManager{std::move(newBufferManager)},
      vertexBufferMaxSize(vertexSize * 10240),
      indexBufferMaxSize(sizeof(uint32_t) * 30240),
      vertexBufferMaxLoad(0.8f),
      indexBufferMaxLoad(0.8f),
      vertexBufferHandle(
          bufferManager->createGpuVertexBuffer(vertexBufferMaxSize,
                                               fmt::format("Buffer-{}-Vertex", bufferName.data()))),
      indexBufferHandle(
          bufferManager->createGpuIndexBuffer(indexBufferMaxSize,
                                              fmt::format("Buffer-{}-Index", bufferName.data()))) {
}

auto MeshBufferManager::addMesh(const IGeometryData& geometryData) -> MeshHandle {
  const auto vertexSize = geometryData.getVertexDataSize();
  const auto indexSize = geometryData.getIndexDataSize();

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
      vertexBufferHandle = bufferManager->resizeBuffer(vertexBufferHandle, calculatedMaxSize);
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
      indexBufferHandle = bufferManager->resizeBuffer(indexBufferHandle, calculatedIndexMaxSize);
      indexBufferMaxSize = calculatedIndexMaxSize;
    }
  }

  bufferManager->addToBuffer(geometryData,
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
  bufferEntries.emplace_back(geometryData.getIndexCount(),
                             indexOffset,
                             vertexOffset,
                             geometryData.getVertexCount());

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

auto MeshBufferManager::getBuffers() const -> std::tuple<Buffer&, Buffer&> {
  return {bufferManager->getBuffer(getVertexBufferHandle()),
          bufferManager->getBuffer(getIndexBufferHandle())};
}

}
