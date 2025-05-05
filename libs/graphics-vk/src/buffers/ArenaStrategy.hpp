#pragma once

#include "buffers/IBufferStrategy.hpp"

namespace tr {

class ArenaStrategy : public IBufferStrategy {
public:
  ArenaStrategy() = default;
  ~ArenaStrategy() override = default;

  ArenaStrategy(const ArenaStrategy&) = default;
  ArenaStrategy(ArenaStrategy&&) = delete;
  auto operator=(const ArenaStrategy&) -> ArenaStrategy& = default;
  auto operator=(ArenaStrategy&&) -> ArenaStrategy& = delete;

  auto insert(ManagedBuffer& buffer, const void* data, size_t size) -> BufferRegion override;

  auto remove(ManagedBuffer& buffer, BufferRegion region) -> void override;
};

}
