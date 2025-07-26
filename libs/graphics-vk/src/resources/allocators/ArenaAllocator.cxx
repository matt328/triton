#include "ArenaAllocator.hpp"

namespace tr {

// TODO(matt): Make this allocator more arena-like. right now it's just a clone of LinearAllocator

ArenaAllocator::ArenaAllocator(Handle<ManagedBuffer> bufferHandle,
                               size_t newInitialSize,
                               std::string newName)
    : IBufferAllocator{bufferHandle}, currentBufferSize{newInitialSize}, name{std::move(newName)} {
}

auto ArenaAllocator::allocate(const BufferRequest& request) -> BufferRegion {
  assert(request.size != 0);

  const auto offset = currentOffset;
  currentOffset += request.size;
  return BufferRegion{
      .offset = offset,
      .size = request.size,
  };
}

auto ArenaAllocator::checkSize(const BufferRequest& requestData) -> std::optional<ResizeRequest> {
  const auto requestedSize = currentOffset + requestData.size;
  if (requestedSize > currentBufferSize) {
    // Currently only make room for one more model so we can test resizing easier
    return ResizeRequest{.bufferHandle = bufferHandle,
                         .newSize = currentBufferSize + requestData.size};
  }
  return std::nullopt;
}

auto ArenaAllocator::freeRegion([[maybe_unused]] const BufferRegion& region) -> void {
}

auto ArenaAllocator::reset() -> void {
  currentOffset = 0;
}

auto ArenaAllocator::notifyBufferResized(size_t newSize) -> void {
  currentBufferSize = newSize;
}

}
