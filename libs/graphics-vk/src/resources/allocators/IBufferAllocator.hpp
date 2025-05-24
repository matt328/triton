#pragma once

namespace tr {

struct BufferRequest {
  size_t size;
};

struct BufferPosition {
  size_t offset;
};

class IBufferAllocator {
public:
  IBufferAllocator() = default;
  virtual ~IBufferAllocator() = default;

  IBufferAllocator(const IBufferAllocator&) = default;
  IBufferAllocator(IBufferAllocator&&) = delete;
  auto operator=(const IBufferAllocator&) -> IBufferAllocator& = default;
  auto operator=(IBufferAllocator&&) -> IBufferAllocator& = delete;

  virtual auto allocate(const BufferRequest& requestData) -> std::optional<BufferPosition> = 0;
  virtual auto reset() -> void = 0;
};

}
