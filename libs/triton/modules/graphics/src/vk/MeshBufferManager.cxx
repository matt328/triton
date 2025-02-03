#include "vk/MeshBufferManager.hpp"

#include "geo/GeometryData.hpp"
#include "vk/BufferManager.hpp"
#include "vk/VkResourceManager.hpp"
#include <ranges>

namespace tr {

MeshBufferManager::MeshBufferManager(std::shared_ptr<IBufferManager> newBufferManager,
                                     size_t vertexSize,
                                     std::string_view bufferName)
    : bufferManager{std::move(newBufferManager)},
      vbStride(vertexSize),
      ibStride(sizeof(uint32_t)),
      vbMaxSize(vertexSize * 10240),
      ibMaxSize(sizeof(uint32_t) * 30240),
      vbMaxLoad(0.8f),
      ibMaxLoad(0.8f),
      vbHandle(
          bufferManager->createGpuVertexBuffer(vbMaxSize,
                                               fmt::format("Buffer-{}-Vertex", bufferName.data()))),
      ibHandle(
          bufferManager->createGpuIndexBuffer(ibMaxSize,
                                              fmt::format("Buffer-{}-Index", bufferName.data()))) {
}

MeshBufferManager::~MeshBufferManager() {
}

auto MeshBufferManager::addMesh(const IGeometryData& geometryData) -> MeshHandle {

  // Default insert position to the 'end'
  auto ibInsertPosition = ibMaxAllocatedOffset;
  {
    // Find an empty block and set insert position there if one is found
    const auto emptyBlock = findEmptyBlock(geometryData, emptyIndexBlocks);
    if (emptyBlock.has_value()) {
      ibInsertPosition = emptyBlock->offset;
    }
  }

  // Same as Index Buffer
  auto vbInsertPosition = vbMaxAllocatedOffset;
  {
    const auto emptyBlock = findEmptyBlock(geometryData, emptyVertexBlocks);
    if (emptyBlock.has_value()) {
      vbInsertPosition = emptyBlock->offset;
    }
  }

  // If we're inserting at the end, check if it will fit, if not, resize
  const auto ibInsertPositionInBytes = ibStride * ibInsertPosition;
  auto ibNeedsResize = ibInsertPositionInBytes + geometryData.getIndexDataSize() > ibMaxSize;
  if (ibInsertPosition == ibMaxAllocatedOffset && ibNeedsResize) {
    resizeIndexBuffer();
  }

  // Same as Index buffer
  const auto vbInsertPositionInBytes = vbStride * vbInsertPosition;
  auto vbNeedsResize = vbInsertPositionInBytes + geometryData.getVertexDataSize() > vbMaxSize;
  if (vbInsertPosition == vbMaxAllocatedOffset && vbNeedsResize) {
    resizeVertexBuffer();
  }

  // Add the data to the buffers at the determined offsets
  bufferManager->addToBuffer(geometryData, vbHandle, vbInsertPosition, ibHandle, ibInsertPosition);

  // If inserted at the end, set the new max offset
  if (ibInsertPosition == ibMaxAllocatedOffset) {
    ibMaxAllocatedOffset += geometryData.getIndexCount();
  }
  // If inserted at the end, set the new max offset
  if (vbInsertPosition == vbMaxAllocatedOffset) {
    vbMaxAllocatedOffset += geometryData.getVertexCount();
  }

  // Insert a bufferEntry
  const auto meshHandle = bufferKeygen.getKey();
  bufferEntries.insert({meshHandle,
                        BufferEntry{.indexCount = geometryData.getIndexCount(),
                                    .indexOffset = static_cast<uint32_t>(ibInsertPosition),
                                    .vertexCount = geometryData.getVertexCount(),
                                    .vertexOffset = static_cast<uint32_t>(vbInsertPosition)}});
  return meshHandle;
}

auto MeshBufferManager::getVertexBufferHandle() const -> BufferHandle {
  return vbHandle;
}

auto MeshBufferManager::getIndexBufferHandle() const -> BufferHandle {
  return ibHandle;
}

auto MeshBufferManager::removeMesh(MeshHandle meshHandle) -> void {
  auto bufferEntry = bufferEntries[meshHandle];
  emptyIndexBlocks.emplace_back(
      Block{.offset = bufferEntry.indexOffset, .size = bufferEntry.indexCount});
  emptyVertexBlocks.emplace_back(
      Block{.offset = bufferEntry.vertexOffset, .size = bufferEntry.vertexCount});
  bufferEntries.erase(meshHandle);
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
                                                .firstIndex = bufferEntry.indexOffset,
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

auto MeshBufferManager::findEmptyBlock(const IGeometryData& geometryData,
                                       const std::vector<Block>& blocks) -> std::optional<Block> {
  for (const auto& block : blocks) {
    if (geometryData.getIndexDataSize() <= block.size) {
      return std::make_optional(block);
    }
  }
  return std::nullopt;
}

}
