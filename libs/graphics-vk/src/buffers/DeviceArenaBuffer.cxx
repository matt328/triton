#include "DeviceArenaBuffer.hpp"

namespace tr {

DeviceArenaBuffer::DeviceArenaBuffer(std::shared_ptr<Device> newDevice,
                                     vk::Buffer newVkBuffer,
                                     std::shared_ptr<vma::Allocator> newAllocator,
                                     vma::Allocation newAllocation,
                                     const DeviceArenaBufferCreateInfo& createInfo)
    : IBuffer(std::move(newDevice), newVkBuffer, std::move(newAllocator), newAllocation),
      capacity(createInfo.initialCapacity),
      itemStride(createInfo.itemStride) {
}

auto DeviceArenaBuffer::setData(const void* data, size_t size) -> BufferRegion {
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

    ZoneNamedN(var, "Resize Buffer", true);

    auto* bci = bufferMeta.bufferCreateInfo;
    const auto oldSize = bci->size;
    bci->size = newCapacity;

    auto* aci = bufferMeta.allocationCreateInfo;

    auto [newBuffer, newAllocation] = allocator->createBuffer(*bci, *aci);

    allocation = newAllocation;

    immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
      ZoneNamedN(var, "Copy Buffer", true);
      const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = oldSize};
      cmd.copyBuffer(vkBuffer, newBuffer, vbCopy);
    });

    {
      ZoneNamedN(var, "Erasing Buffer", true);
      unusedBuffers.push_back(handle);
    }
    TracyMessageL("Old Buffer Erased");
    const auto newHandle = bufferMapKeygen.getKey();
    bufferMap.emplace(newHandle, std::move(newBuffer));
    return newHandle;

    capacity = newCapacity;
  }

  bufferManager->addToSingleBuffer(data, size, bufferHandle, insertPosition * itemStride);

  if (insertPosition == maxOffset) {
    maxOffset += itemCount;
  }

  return BufferRegion{.offset = insertPosition, .size = size};
}

auto DeviceArenaBuffer::mergeRegionWithNeighbors(const RegionContainer::iterator& it) -> void {
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
