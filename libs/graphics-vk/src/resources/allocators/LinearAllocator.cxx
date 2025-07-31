#include "LinearAllocator.hpp"

namespace tr {

LinearAllocator::LinearAllocator(Handle<ManagedBuffer> bufferHandle,
                                 size_t bufferSize,
                                 std::string newName)
    : IBufferAllocator{bufferHandle}, maxBufferSize{bufferSize}, name{std::move(newName)} {
}

auto LinearAllocator::allocate(const BufferRequest& bufferRequest) -> BufferRegion {
  const auto oldOffset = currentOffset;
  currentOffset += bufferRequest.size;
  return BufferRegion{.offset = oldOffset, .size = bufferRequest.size};
}

auto LinearAllocator::checkSize(const BufferRequest& requestData) -> std::optional<ResizeRequest> {
  if (currentOffset + requestData.size > maxBufferSize) {
    Log.warn("Allocator: {}, current buffer size={} + requested size={} ({})> maxBufferSize={}",
             name,
             currentOffset,
             requestData.size,
             currentOffset + requestData.size,
             maxBufferSize);
    return ResizeRequest{.bufferHandle = bufferHandle, .newSize = currentOffset + requestData.size};
  }
  return std::nullopt;
}

auto LinearAllocator::freeRegion([[maybe_unused]] const BufferRegion& region) -> void {
}

auto LinearAllocator::reset() -> void {
  currentOffset = 0;
}

auto LinearAllocator::notifyBufferResized(size_t newSize) -> void {
  maxBufferSize = newSize;
}

}
