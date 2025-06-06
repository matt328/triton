#include "ArenaAllocator.hpp"

namespace tr {

ArenaAllocator::ArenaAllocator(size_t newInitialSize) : initialSize{newInitialSize} {
}

auto ArenaAllocator::allocate([[maybe_unused]] const BufferRequest& requestData)
    -> std::optional<BufferRegion> {
  return std::nullopt;
}

auto ArenaAllocator::freeRegion([[maybe_unused]] const BufferRegion& region) -> void {
}

auto ArenaAllocator::reset() -> void {
}

}
