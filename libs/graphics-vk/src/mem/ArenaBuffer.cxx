#include "ArenaBuffer.hpp"

namespace tr {

ArenaBuffer::ArenaBuffer(IBufferManager* newBufferManager, ArenaBufferCreateInfo createInfo)
    : bufferManager{newBufferManager},
      capacity(createInfo.initialBufferSize),
      itemStride{createInfo.newItemStride} {
  if (createInfo.bufferType == ArenaBufferType::Vertex) {
    bufferHandle = bufferManager->createGpuVertexBuffer(
        capacity,
        fmt::format("Buffer-{}-Vertex", createInfo.bufferName.data()));
  } else if (createInfo.bufferType == ArenaBufferType::Index) {
    bufferHandle = bufferManager->createGpuIndexBuffer(
        capacity,
        fmt::format("Buffer-{}-Vertex", createInfo.bufferName.data()));
  }
}

ArenaBuffer::~ArenaBuffer() {
  Log.trace("Destroying Arena Buffer");
}

auto ArenaBuffer::insertData(const void* data, size_t size) -> BufferRegion {

  auto insertPosition = maxOffset;
  auto itemCount = (size / itemStride);

  auto regionIt = freeList.lower_bound(BufferRegion{.offset = 0, .size = size});

  if (regionIt != freeList.end()) {
    insertPosition = regionIt->offset;

    // Either shrink or remove the no longer empty region
    auto newRegionSize = regionIt->size - itemCount;

    if (newRegionSize == 0) {
      freeList.erase(regionIt);
    } else {
      auto newBufferRegion =
          BufferRegion{.offset = regionIt->offset + itemCount, .size = newRegionSize};
      freeList.erase(regionIt);
      freeList.insert(newBufferRegion);
    }
  }

  const auto insertPositionBytes = itemStride * insertPosition;
  auto bufferNeedsResize = insertPositionBytes + size > capacity;

  if (insertPosition == maxOffset && bufferNeedsResize) {
    auto newCapacity = capacity;
    while (newCapacity < capacity + size) {
      newCapacity *= 2;
    }
    Log.trace("Size: {}, current capacity: {}, new capacity: {}", size, capacity, newCapacity);
    bufferHandle = bufferManager->resizeBuffer(bufferHandle, newCapacity);
    capacity = newCapacity;
  }

  bufferManager->addToSingleBuffer(data, size, bufferHandle, insertPosition * itemStride);

  if (insertPosition == maxOffset) {
    maxOffset += itemCount;
  }

  return BufferRegion{.offset = insertPosition, .size = size};
}

auto ArenaBuffer::removeData(const BufferRegion& bufferIndex) -> void {
  bufferManager->removeData(bufferHandle, bufferIndex.offset, bufferIndex.size);
  auto [it, s] = freeList.insert(bufferIndex);
  mergeWithNeighbors(it);
}

auto ArenaBuffer::getBuffer() const -> Buffer& {
  return bufferManager->getBuffer(bufferHandle);
}

auto ArenaBuffer::mergeWithNeighbors(const RegionContainer::iterator& it) -> void {

  if (it == freeList.end()) {
    return;
  }

  auto prev = (it == freeList.begin()) ? freeList.end() : std::prev(it);
  auto next = std::next(it);

  auto mergedBlock = *it;

  if (prev != freeList.end() && prev->offset + prev->size == it->offset) {
    mergedBlock.offset = prev->offset;
    mergedBlock.size += prev->size;
    freeList.erase(prev); // Avoid reinsertion
  }

  if (next != freeList.end() && it->offset + it->size == next->offset) {
    mergedBlock.size += next->size;
    freeList.erase(next);
  }

  if (mergedBlock.size != it->size) { // Only modify if merging happened
    freeList.erase(it);
    freeList.insert(mergedBlock);
  }
}

}
