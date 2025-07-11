#pragma once

#include "mem/BufferRegion.hpp"
namespace tr {

struct BufferRequest {
  size_t size;
};

class IBufferAllocator {
public:
  IBufferAllocator() = default;
  virtual ~IBufferAllocator() = default;

  IBufferAllocator(const IBufferAllocator&) = default;
  IBufferAllocator(IBufferAllocator&&) = delete;
  auto operator=(const IBufferAllocator&) -> IBufferAllocator& = default;
  auto operator=(IBufferAllocator&&) -> IBufferAllocator& = delete;

  virtual auto allocate(const BufferRequest& requestData) -> std::optional<BufferRegion> = 0;
  virtual auto needsResize() -> bool = 0;
  [[nodiscard]] virtual auto getRecommendedSize() const -> size_t = 0;
  virtual auto notifyBufferResized(size_t newSize) -> void = 0;
  virtual auto freeRegion(const BufferRegion& region) -> void = 0;
  virtual auto reset() -> void = 0;
};

}
