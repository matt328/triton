#include "ArenaBuffer.hpp"

namespace tr {

ArenaBuffer::ArenaBuffer(std::shared_ptr<IBufferManager> newBufferManager,
                         size_t newItemStride,
                         size_t initialBufferSize,
                         std::string_view bufferName)
    : bufferManager{std::move(newBufferManager)},
      capacity(initialBufferSize),
      itemStride{newItemStride} {
  bufferHandle =
      bufferManager->createGpuVertexBuffer(capacity,
                                           fmt::format("Buffer-{}-Vertex", bufferName.data()));
}

ArenaBuffer::~ArenaBuffer() {
  Log.trace("Destroying Arena Buffer");
}

auto ArenaBuffer::insertData([[maybe_unused]] void* data, size_t size) -> BufferRegion {

  auto insertPosition = maxOffset;
  auto itemCount = (size / itemStride);

  auto region = findEmptyRegion(size);
  if (region.has_value()) {
    insertPosition = region.value().get().offset;

    // Either shrink or remove the no longer empty region
    auto newRegionSize = region.value().get().size - itemCount;

    if (newRegionSize == 0) {
      auto it = std::find(freeList.begin(), freeList.end(), *region);
      freeList.erase(it);
    } else {
      auto newOffset = region.value().get().offset + itemCount;
      region.value().get().offset = newOffset;
      region.value().get().size = newRegionSize;
    }
  }

  const auto insertPositionBytes = itemStride * insertPosition;
  auto bufferNeedsResize = insertPositionBytes + size > capacity;

  if (insertPosition == maxOffset && bufferNeedsResize) {
    auto newCapacity = capacity;
    while (newCapacity < capacity + size) {
      newCapacity *= 2;
    }
    bufferHandle = bufferManager->resizeBuffer(bufferHandle, newCapacity);
    capacity = newCapacity;
  }

  bufferManager->addToSingleBuffer(data, size, bufferHandle, insertPosition * itemStride);

  if (insertPosition == maxOffset) {
    maxOffset += itemCount;
  }

  return BufferRegion{.offset = insertPosition, .size = size};
}

auto ArenaBuffer::removeData([[maybe_unused]] const BufferRegion& bufferIndex) -> void {
}

auto ArenaBuffer::getBuffer() const -> Buffer& {
  return bufferManager->getBuffer(bufferHandle);
}

auto ArenaBuffer::findEmptyRegion(size_t requiredSize)
    -> std::optional<std::reference_wrapper<BufferRegion>> {

  auto it = freeList.lower_bound(BufferRegion{.offset = 0, .size = requiredSize});
  if (it == freeList.end()) {
    return std::nullopt;
  }

  return std::ref(const_cast<BufferRegion&>(*it));
}

auto ArenaBuffer::mergeWithNeighbors(RegionContainer::iterator it) -> void {

  if (it == freeList.end()) {
    return;
  }

  auto prev = (it == freeList.begin()) ? freeList.end() : std::prev(it);
  auto next = std::next(it);

  bool merged = false;
  auto mergedBlock = *it;

  if (prev != freeList.end() && prev->offset + prev->size == it->offset) {
    mergedBlock.offset = prev->offset;
    mergedBlock.size += prev->size;
    freeList.erase(prev);
    merged = true;
  }

  if (next != freeList.end() && it->offset + it->size == next->offset) {
    mergedBlock.size += next->size;
    freeList.erase(next);
    merged = true;
  }

  if (merged) {
    freeList.erase(it);
    freeList.insert(mergedBlock);
  }
}

}
