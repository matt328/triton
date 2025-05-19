#include "ArenaStrategy.hpp"

namespace tr {

auto ArenaStrategy::insert([[maybe_unused]] ManagedBuffer& buffer,
                           [[maybe_unused]] const void* data,
                           [[maybe_unused]] size_t size) -> BufferRegion {
  return {.offset = 0, .size = size};
}

auto ArenaStrategy::remove([[maybe_unused]] ManagedBuffer& buffer,
                           [[maybe_unused]] BufferRegion region) -> void {
}

}
