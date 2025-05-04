#pragma once

#include "mem/ArenaBuffer.hpp"
#include "mem/Buffer.hpp"
#include "vk/ArenaGeometryBuffer.hpp"

namespace tr {

class BufferWrapper {
public:
  ~BufferWrapper() {
  }

  BufferWrapper(const BufferWrapper&) = delete;
  BufferWrapper(BufferWrapper&&) = delete;
  auto operator=(const BufferWrapper&) -> BufferWrapper& = delete;
  auto operator=(BufferWrapper&&) -> BufferWrapper& = delete;

  template <typename T, typename... Args>
  static auto create(Args&&... args) {
    return std::unique_ptr<BufferWrapper>(
        new BufferWrapper(std::in_place_type<T>, std::forward<Args>(args)...));
  }

  template <typename T>
  auto get() -> T* {
    return std::get_if<T>(&buffer);
  }

  template <typename T>
  auto get() const -> const T* {
    return std::get_if<T>(&buffer);
  }

private:
  template <typename T, typename... Args>
  explicit BufferWrapper(std::in_place_type_t<T>, Args&&... args)
      : buffer(std::in_place_type<T>, std::forward<Args>(args)...) {
  }
  std::variant<ManagedBuffer, ArenaBuffer, ArenaGeometryBuffer> buffer;
};

}
