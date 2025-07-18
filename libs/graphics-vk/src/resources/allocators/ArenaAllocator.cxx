#include "ArenaAllocator.hpp"

namespace tr {

ArenaAllocator::ArenaAllocator(size_t newInitialSize, std::string newName)
    : currentBufferSize{newInitialSize}, name{std::move(newName)} {
}

auto ArenaAllocator::allocate(const BufferRequest& request) -> std::optional<BufferRegion> {
  if (request.size == 0) {
    return std::nullopt;
  }

  if (currentOffset + request.size > currentBufferSize) {
    resizeRequired = true;
    lastFailedRequestSize = request.size;
    Log.warn("Allocator failed, requiredSize={}, currentBufferSize={}",
             currentOffset + request.size,
             currentBufferSize);
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
