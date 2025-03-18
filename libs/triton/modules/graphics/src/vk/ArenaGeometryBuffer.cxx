#include "vk/ArenaGeometryBuffer.hpp"
#include "cm/IGeometryData.hpp"
#include "mem/ArenaBuffer.hpp"

namespace tr {

ArenaGeometryBuffer::ArenaGeometryBuffer(std::shared_ptr<IBufferManager> newBufferManager,
                                         const ArenaGeometryBufferCreateInfo& createInfo)
    : bufferManager{std::move(newBufferManager)} {

  const auto vbAci = ArenaBufferCreateInfo{
      .newItemStride = createInfo.vertexSize,
      .initialBufferSize = 8192000,
      .bufferType = BufferType::Vertex,
      .bufferName = createInfo.bufferName,
  };
  vertexBuffer = std::make_unique<ArenaBuffer>(bufferManager, vbAci);

  const auto ibAci = ArenaBufferCreateInfo{
      .newItemStride = createInfo.indexSize,
      .initialBufferSize = 819200,
      .bufferType = BufferType::Index,
      .bufferName = createInfo.bufferName,
  };
  indexBuffer = std::make_unique<ArenaBuffer>(bufferManager, ibAci);
}

ArenaGeometryBuffer::~ArenaGeometryBuffer() {
  Log.trace("Destroying ArenaGeometryBuffer");
}

auto ArenaGeometryBuffer::addMesh(const IGeometryData& geometryData) -> MeshHandle {
  const auto vbRegion =
      vertexBuffer->insertData(geometryData.getVertexData(), geometryData.getVertexDataSize());

  const auto ibRegion =
      indexBuffer->insertData(geometryData.getIndexData(), geometryData.getIndexDataSize());

  const auto meshHandle = bufferKeygen.getKey();
  const auto bufferEntry = BufferEntry{.indexCount = geometryData.getIndexCount(),
                                       .indexOffset = static_cast<uint32_t>(ibRegion.offset),
                                       .indexSize = static_cast<uint32_t>(ibRegion.size),
                                       .vertexCount = geometryData.getVertexCount(),
                                       .vertexOffset = static_cast<uint32_t>(vbRegion.offset),
                                       .vertexSize = static_cast<uint32_t>(vbRegion.size)};
  bufferEntries.insert({meshHandle, bufferEntry});
  return meshHandle;
}

auto ArenaGeometryBuffer::removeMesh(MeshHandle meshHandle) {
  const auto bufferEntry = bufferEntries.at(meshHandle);

  const auto vbRegion =
      BufferRegion{.offset = bufferEntry.vertexOffset, .size = bufferEntry.vertexSize};
  const auto ibRegion =
      BufferRegion{.offset = bufferEntry.indexOffset, .size = bufferEntry.indexSize};

  vertexBuffer->removeData(vbRegion);
  indexBuffer->removeData(ibRegion);
  bufferEntries.erase(meshHandle);
}

auto ArenaGeometryBuffer::getGpuBufferEntries(const std::vector<RenderMeshData>& meshDataList)
    -> std::vector<GpuBufferEntry>& {

  gpuBufferEntryCache.clear();

  for (const auto& meshData : meshDataList) {
    if (bufferEntries.contains(meshData.handle)) {
      const auto& bufferEntry = bufferEntries.at(meshData.handle);
      gpuBufferEntryCache.push_back(GpuBufferEntry{.indexCount = bufferEntry.indexCount,
                                                   .firstIndex = bufferEntry.indexOffset,
                                                   .vertexOffset = bufferEntry.vertexOffset,
                                                   .instanceCount = 1,
                                                   .firstInstance = 0,
                                                   .padding = 0});
    }
  }

  return gpuBufferEntryCache;
}

auto ArenaGeometryBuffer::getBuffers() const -> std::tuple<Buffer&, Buffer&> {
  return {vertexBuffer->getBuffer(), indexBuffer->getBuffer()};
}

}
