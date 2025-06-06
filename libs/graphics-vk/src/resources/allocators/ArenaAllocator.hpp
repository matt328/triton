#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class ArenaAllocator : public IBufferAllocator {
public:
  explicit ArenaAllocator(size_t newInitialSize);
  ~ArenaAllocator() override = default;

  ArenaAllocator(const ArenaAllocator&) = default;
  ArenaAllocator(ArenaAllocator&&) = delete;
  auto operator=(const ArenaAllocator&) -> ArenaAllocator& = default;
  auto operator=(ArenaAllocator&&) -> ArenaAllocator& = delete;

  auto allocate(const BufferRequest& requestData) -> std::optional<BufferRegion> override;
  auto freeRegion(const BufferRegion& region) -> void override;
  auto reset() -> void override;

private:
  [[maybe_unused]] size_t initialSize{};
};

}
