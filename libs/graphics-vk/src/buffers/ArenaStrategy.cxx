#include "ArenaStrategy.hpp"

namespace tr {

auto ArenaStrategy::insert(ManagedBuffer& buffer, const void* data, size_t size) -> BufferRegion {
  return {.offset = 0, .size = size};
}

auto ArenaStrategy::remove(ManagedBuffer& buffer, BufferRegion region) -> void {
}

}
