#include "LinearAllocator.hpp"

namespace tr {

LinearAllocator::LinearAllocator(size_t bufferSize) : maxBufferSize{bufferSize} {
}

auto LinearAllocator::allocate(const BufferRequest& bufferRequest)
    -> std::optional<BufferPosition> {
  if (currentOffset + bufferRequest.size > maxBufferSize) {
    return std::nullopt;
  }

  auto oldOffset = currentOffset;
  currentOffset += bufferRequest.size;

  return std::make_optional<BufferPosition>(BufferPosition{.offset = oldOffset});
}

auto LinearAllocator::reset() -> void {
  currentOffset = 0;
}

}
