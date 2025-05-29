#include "LinearAllocator.hpp"

namespace tr {

LinearAllocator::LinearAllocator(size_t bufferSize) : maxBufferSize{bufferSize} {
}

auto LinearAllocator::allocate(const BufferRequest& bufferRequest) -> std::optional<BufferRegion> {
  if (currentOffset + bufferRequest.size > maxBufferSize) {
    Log.warn("Requested buffer size={}, maxBufferSize={}", bufferRequest.size, maxBufferSize);
    return std::nullopt;
  }

  auto oldOffset = currentOffset;
  currentOffset += bufferRequest.size;

  return std::make_optional<BufferRegion>(
      BufferRegion{.offset = oldOffset, .size = bufferRequest.size});
}

auto LinearAllocator::freeRegion([[maybe_unused]] const BufferRegion& region) -> void {
}

auto LinearAllocator::reset() -> void {
  currentOffset = 0;
}

}
