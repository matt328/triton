#pragma once

#include "buffers/ManagedBuffer.hpp"
#include "mem/BufferRegion.hpp"

namespace tr {

class IBufferStrategy {
public:
  IBufferStrategy() = default;
  virtual ~IBufferStrategy() = default;

  IBufferStrategy(const IBufferStrategy&) = default;
  IBufferStrategy(IBufferStrategy&&) = delete;
  auto operator=(const IBufferStrategy&) -> IBufferStrategy& = default;
  auto operator=(IBufferStrategy&&) -> IBufferStrategy& = delete;

  virtual void rewrite([[maybe_unused]] ManagedBuffer& buffer,
                       [[maybe_unused]] const void* data,
                       [[maybe_unused]] size_t size) {
    throw std::runtime_error("Rewrite not supported for this buffer");
  }

  virtual auto insert([[maybe_unused]] ManagedBuffer& buffer,
                      [[maybe_unused]] const void* data,
                      [[maybe_unused]] size_t size) -> BufferRegion {
    throw std::runtime_error("Insert not supported for this buffer");
  }

  virtual void remove([[maybe_unused]] ManagedBuffer& buffer,
                      [[maybe_unused]] BufferRegion region) {
    throw std::runtime_error("Remove not supported for this buffer");
  }
};

}
