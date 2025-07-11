#include "ArenaAllocator.hpp"

namespace tr {

ArenaAllocator::ArenaAllocator(size_t newInitialSize) : currentBufferSize{newInitialSize} {
}

auto ArenaAllocator::allocate([[maybe_unused]] const BufferRequest& request)
    -> std::optional<BufferRegion> {
  if (request.size == 0) {
    return std::nullopt;
  }

  if (currentOffset + request.size > currentBufferSize) {
    resizeRequired = true;
    lastFailedRequestSize = request.size;
    return std::nullopt;
  }

  const auto offset = currentOffset;
  currentOffset += request.size;
  return BufferRegion{
      .offset = offset,
      .size = request.size,
  };
}

auto ArenaAllocator::freeRegion([[maybe_unused]] const BufferRegion& region) -> void {
}

auto ArenaAllocator::reset() -> void {
  currentOffset = 0;
  resizeRequired = false;
  lastFailedRequestSize = 0;
}

auto ArenaAllocator::needsResize() -> bool {
  return resizeRequired;
}

auto ArenaAllocator::getRecommendedSize() const -> size_t {
  const auto required = currentOffset + lastFailedRequestSize;
  const auto doubled = std::max(required, currentBufferSize * 2);
  return doubled;
}

auto ArenaAllocator::notifyBufferResized(size_t newSize) -> void {
  currentBufferSize = newSize;
  resizeRequired = false;
  lastFailedRequestSize = 0l;
}

}
