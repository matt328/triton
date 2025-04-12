#pragma once

#include "mem/ArenaBuffer.hpp"
#include "mem/Buffer.hpp"
#include "vk/ArenaGeometryBuffer.hpp"

namespace tr {

class BufferWrapper {
public:
  template <typename T>
  explicit BufferWrapper(T&& b) : buffer(b) {
  }
  ~BufferWrapper() = default;

  BufferWrapper(const BufferWrapper&) = delete;
  BufferWrapper(BufferWrapper&&) = delete;
  auto operator=(const BufferWrapper&) -> BufferWrapper& = delete;
  auto operator=(BufferWrapper&&) -> BufferWrapper& = delete;

  template <typename T>
  auto get() -> T* {
    return std::get_if<T>(&buffer);
  }

  template <typename T>
  auto get() const -> const T* {
    return std::get_if<T>(&buffer);
  }

private:
  std::variant<Buffer, ArenaBuffer, ArenaGeometryBuffer> buffer;
};

}
