#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class LinearAllocator : public IBufferAllocator {
public:
  explicit LinearAllocator(size_t bufferSize);
  ~LinearAllocator() override = default;

  LinearAllocator(const LinearAllocator&) = default;
  LinearAllocator(LinearAllocator&&) = delete;
  auto operator=(const LinearAllocator&) -> LinearAllocator& = default;
  auto operator=(LinearAllocator&&) -> LinearAllocator& = delete;

  auto allocate(const BufferRequest& bufferRequest) -> std::optional<BufferRegion> override;
  auto needsResize() -> bool override;
  [[nodiscard]] auto getRecommendedSize() const -> size_t override;
  auto notifyBufferResized(size_t newSize) -> void override;
  auto freeRegion(const BufferRegion& region) -> void override;
  auto reset() -> void override;

private:
  size_t currentOffset{};
  size_t maxBufferSize{};
};

}
