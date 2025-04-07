#pragma once

namespace tr {

class BufferRegistry {
public:
  BufferRegistry() = default;
  ~BufferRegistry() = default;

  BufferRegistry(const BufferRegistry&) = default;
  BufferRegistry(BufferRegistry&&) = delete;
  auto operator=(const BufferRegistry&) -> BufferRegistry& = default;
  auto operator=(BufferRegistry&&) -> BufferRegistry& = delete;
};

}
