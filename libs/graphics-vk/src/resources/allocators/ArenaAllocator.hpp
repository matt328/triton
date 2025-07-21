#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class ArenaAllocator : public IBufferAllocator {
public:
  ArenaAllocator(Handle<ManagedBuffer> bufferHandle, size_t newInitialSize, std::string newName);
  ~ArenaAllocator() override = default;

  ArenaAllocator(const ArenaAllocator&) = default;
  ArenaAllocator(ArenaAllocator&&) = delete;
  auto operator=(const ArenaAllocator&) -> ArenaAllocator& = default;
  auto operator=(ArenaAllocator&&) -> ArenaAllocator& = delete;

  auto allocate(const BufferRequest& requestData) -> BufferRegion override;
  auto checkSize(const BufferRequest& requestData) -> std::optional<ResizeRequest> override;
  auto notifyBufferResized(size_t newSize) -> void override;
  auto freeRegion(const BufferRegion& region) -> void override;
  auto reset() -> void override;

private:
  size_t currentBufferSize{};
  size_t currentOffset = 0;

  std::string name;
};

}
