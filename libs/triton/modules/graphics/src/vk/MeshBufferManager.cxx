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
    auto emptyBlock = findEmptyBlock(geometryData.getIndexCount(), emptyIndexBlocks);
    if (emptyBlock.has_value()) {
      ibInsertPosition = emptyBlock.value().get().offset;
      // Either Shrink or remove the empty block
      auto newEmptyBlockSize = emptyBlock.value().get().size - geometryData.getIndexCount();
      if (newEmptyBlockSize == 0) {
        auto it = std::find(emptyIndexBlocks.begin(), emptyIndexBlocks.end(), *emptyBlock);
        emptyIndexBlocks.erase(it);
      } else {
        auto newEmptyBlockOffset = emptyBlock.value().get().offset + geometryData.getIndexCount();
        emptyBlock.value().get().offset = newEmptyBlockOffset;
        emptyBlock.value().get().size = newEmptyBlockSize;
      }
    }
  }

  // Same as Index Buffer
  auto vbInsertPosition = vbMaxAllocatedOffset;
  {
    auto emptyBlock = findEmptyBlock(geometryData.getVertexCount(), emptyVertexBlocks);
    if (emptyBlock.has_value()) {
      vbInsertPosition = emptyBlock.value().get().offset;
      // Either Shrink or remove the empty block
      auto newEmptyBlockSize = emptyBlock.value().get().size - geometryData.getVertexCount();
      if (newEmptyBlockSize == 0) {
        auto it = std::find(emptyVertexBlocks.begin(), emptyVertexBlocks.end(), *emptyBlock);
        emptyVertexBlocks.erase(it);
      } else {
        auto newEmptyBlockOffset = emptyBlock.value().get().offset + geometryData.getVertexCount();
        emptyBlock.value().get().offset = newEmptyBlockOffset;
        emptyBlock.value().get().size = newEmptyBlockSize;
      }
    }
  }

  // If we're inserting at the end, check if it will fit, if not, resize
  const auto ibInsertPositionInBytes = ibStride * ibInsertPosition;
  auto ibNeedsResize = ibInsertPositionInBytes + geometryData.getIndexDataSize() > ibMaxSize;
  if (ibInsertPosition == ibMaxAllocatedOffset && ibNeedsResize) {
    auto newSize = ibMaxSize;
    while (newSize < ibMaxSize + geometryData.getIndexDataSize()) {
      newSize *= 2;
    }
    ibHandle = bufferManager->resizeBuffer(ibHandle, newSize);
    ibMaxSize = newSize;
  }

  // Same as Index buffer
  const auto vbInsertPositionInBytes = vbStride * vbInsertPosition;
  auto vbNeedsResize = vbInsertPositionInBytes + geometryData.getVertexDataSize() > vbMaxSize;
  if (vbInsertPosition == vbMaxAllocatedOffset && vbNeedsResize) {
    auto newSize = vbMaxSize;
    while (newSize < vbMaxSize + geometryData.getVertexDataSize()) {
      newSize *= 2;
    }
    vbHandle = bufferManager->resizeBuffer(vbHandle, newSize);
    vbMaxSize = newSize;
  }

  // Add the data to the buffers at the determined offsets
  bufferManager->addToBuffer(geometryData,
                             vbHandle,
                             vbInsertPosition * vbStride,
                             ibHandle,
                             ibInsertPosition * ibStride);

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

  auto [it, inserted] =
      emptyIndexBlocks.insert(Block{bufferEntry.indexOffset, bufferEntry.indexCount});
  if (inserted) {
    mergeWithNeighbors(it, emptyIndexBlocks);
  }

  auto [itV, insertedV] =
      emptyVertexBlocks.insert(Block{bufferEntry.vertexOffset, bufferEntry.vertexCount});
  if (insertedV) {
    mergeWithNeighbors(itV, emptyVertexBlocks);
  }

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
    if (bufferEntries.contains(meshData.handle)) {
      const auto& bufferEntry = bufferEntries.at(meshData.handle);
      gpuBufferEntryList.push_back(GpuBufferEntry{.indexCount = bufferEntry.indexCount,
                                                  .firstIndex = bufferEntry.indexOffset,
                                                  .vertexOffset = bufferEntry.vertexOffset,
                                                  .instanceCount = 1,
                                                  .firstInstance = 0,
                                                  .padding = 0});
    }
  }

  return gpuBufferEntryList;
}

auto MeshBufferManager::getBuffers() const -> std::tuple<Buffer&, Buffer&> {
  return {bufferManager->getBuffer(getVertexBufferHandle()),
          bufferManager->getBuffer(getIndexBufferHandle())};
}

auto MeshBufferManager::mergeWithNeighbors(BlockContainer::iterator it, BlockContainer& blocks)
    -> void {
  if (it == blocks.end()) {
    return;
  }

  auto prev = (it == blocks.begin()) ? blocks.end() : std::prev(it);
  auto next = std::next(it);

  bool merged = false;
  Block mergedBlock = *it;

  if (prev != blocks.end() && prev->offset + prev->size == it->offset) {
    mergedBlock.offset = prev->offset;
    mergedBlock.size += prev->size;
    blocks.erase(prev);
    merged = true;
  }

  if (next != blocks.end() && it->offset + it->size == next->offset) {
    mergedBlock.size += next->size;
    blocks.erase(next);
    merged = true;
  }

  if (merged) {
    blocks.erase(it);
    blocks.insert(mergedBlock);
  }
}

auto MeshBufferManager::findEmptyBlock(uint32_t requiredSize, BlockContainer& blocks)
    -> std::optional<std::reference_wrapper<Block>> {

  auto it = blocks.lower_bound(Block{.offset = 0, .size = requiredSize});
  if (it == blocks.end()) {
    return std::nullopt;
  }
  return std::ref(const_cast<Block&>(*it));
}

auto MeshBufferManager::mergeFreeBlocks(BlockContainer& blocks) -> void {

  if (blocks.empty()) {
    return;
  }

  auto it = blocks.begin();
  while (it != blocks.end()) {
    auto next = std::next(it);
    if (next != blocks.end() && it->offset + it->size == next->offset) {
      // Merge blocks
      Block mergedBlock{.offset = it->offset, .size = it->size + next->size};

      // Erase both old blocks and insert merged block
      it = blocks.erase(it);
      it = blocks.erase(it);
      it = blocks.insert(it, mergedBlock);
    } else {
      ++it;
    }
  }
}

}
