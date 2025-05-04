#include "BufferSystem.hpp"
#include "buffers/BufferRequest.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IFrameManager.hpp"
#include "mem/Allocator.hpp"
#include "task/Frame.hpp"

namespace tr {

BufferSystem::BufferSystem(std::shared_ptr<IFrameManager> newFrameManager,
                           std::shared_ptr<Device> newDevice,
                           std::shared_ptr<Allocator> newAllocator)
    : frameManager{std::move(newFrameManager)},
      device{std::move(newDevice)},
      allocator{std::move(newAllocator)} {
}

auto BufferSystem::registerBuffer(BufferRequest& request) -> Handle<ManagedBuffer> {
  const auto handle = bufferHandleGenerator.requestHandle();

  const auto [buffer, bufferMeta] =
      allocator->createBuffer2(&request.bufferCreateInfo, &request.allocationCreateInfo);

  bufferMap.emplace(handle, buffer);
  bufferMetaMap.emplace(handle, bufferMeta);

  return handle;
}

auto BufferSystem::registerPerFrameBuffer(BufferRequest& request) -> LogicalHandle<ManagedBuffer> {
  const auto logicalHandle = bufferHandleGenerator.requestLogicalHandle();

  for (const auto& frame : frameManager->getFrames()) {
    const auto handle = registerBuffer(request);
    frame->addLogicalBuffer(logicalHandle, handle);
  }

  return logicalHandle;
}

auto BufferSystem::updateData(Handle<ManagedBuffer> handle, const void* data, size_t size)
    -> BufferRegion {
  auto& buffer = bufferMap.at(handle);
  auto& bufferMeta = bufferMetaMap.at(handle);
  if (!bufferMeta.arenaInfo) {
    overwriteEntireBuffer(buffer, data, size);
    return BufferRegion{
        .offset = 0,
        .size = size,
    };
  }
  return insertData(buffer, bufferMeta, data, size);
}

auto BufferSystem::removeData(Handle<ManagedBuffer> handle, const BufferRegion& region) -> void {
  auto& bufferMeta = bufferMetaMap.at(handle);
  assert(bufferMeta.arenaInfo && "Tried to remove data from a non-arena buffer");
  auto [it, s] = bufferMeta.arenaInfo->freeList.insert(region);
  mergeWithNeighbors(bufferMeta.arenaInfo->freeList, it);
}

auto BufferSystem::overwriteEntireBuffer(ManagedBuffer& buffer, const void* data, size_t size)
    -> void {
}

auto BufferSystem::insertData(ManagedBuffer& buffer,
                              BufferMeta& meta,
                              const void* data,
                              size_t size) -> BufferRegion {
  if (buffer.isMappable()) {
    // Map the memory and just copy in here.
  } else {
  }
}

auto BufferSystem::mergeWithNeighbors(RegionContainer& freeList,
                                      const RegionContainer::iterator& it) -> void {

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
