#include <utility>

#include "LinearAllocator.hpp"

namespace tr {

LinearAllocator::LinearAllocator(size_t bufferSize, std::string newName)
    : maxBufferSize{bufferSize}, name{std::move(newName)} {
}

auto LinearAllocator::allocate(const BufferRequest& bufferRequest) -> std::optional<BufferRegion> {
  if (currentOffset + bufferRequest.size > maxBufferSize) {
    Log.warn("Allocator: {}, current buffer size={} + requested size={} ({})> maxBufferSize={}",
             name,
             currentOffset,
             bufferRequest.size,
             currentOffset + bufferRequest.size,
             maxBufferSize);
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

auto LinearAllocator::needsResize() -> bool {
  return false;
}

auto LinearAllocator::getRecommendedSize() const -> size_t {
  return maxBufferSize;
}

auto LinearAllocator::notifyBufferResized(size_t newSize) -> void {
}

}
