#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class ArenaAllocator : public IBufferAllocator {
public:
  explicit ArenaAllocator(size_t newInitialSize, std::string newName);
  ~ArenaAllocator() override = default;

  ArenaAllocator(const ArenaAllocator&) = default;
  ArenaAllocator(ArenaAllocator&&) = delete;
  auto operator=(const ArenaAllocator&) -> ArenaAllocator& = default;
  auto operator=(ArenaAllocator&&) -> ArenaAllocator& = delete;

  auto allocate(const BufferRequest& requestData) -> std::optional<BufferRegion> override;
  auto needsResize() -> bool override;
  [[nodiscard]] auto getRecommendedSize() const -> size_t override;
  auto notifyBufferResized(size_t newSize) -> void override;
  auto freeRegion(const BufferRegion& region) -> void override;
  auto reset() -> void override;

private:
  size_t currentBufferSize{};
  size_t currentOffset = 0;

  std::string name;

  bool resizeRequired = false;
  size_t lastFailedRequestSize = 0;
};

}
