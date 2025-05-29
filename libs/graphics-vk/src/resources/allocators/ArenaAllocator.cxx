#include "ArenaAllocator.hpp"

namespace tr {

ArenaAllocator::ArenaAllocator(size_t newInitialSize) : initialSize{newInitialSize} {
}

auto ArenaAllocator::allocate(const BufferRequest& requestData) -> std::optional<BufferRegion> {
  return std::nullopt;
}

auto ArenaAllocator::freeRegion(const BufferRegion& region) -> void {
}

auto ArenaAllocator::reset() -> void {
}

}
