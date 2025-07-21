#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class LinearAllocator : public IBufferAllocator {
public:
  LinearAllocator(Handle<ManagedBuffer> bufferHandle, size_t bufferSize, std::string newName);
  ~LinearAllocator() override = default;

  LinearAllocator(const LinearAllocator&) = default;
  LinearAllocator(LinearAllocator&&) = delete;
  auto operator=(const LinearAllocator&) -> LinearAllocator& = default;
  auto operator=(LinearAllocator&&) -> LinearAllocator& = delete;

  auto allocate(const BufferRequest& bufferRequest) -> BufferRegion override;
  auto checkSize(const BufferRequest& requestData) -> std::optional<ResizeRequest> override;
  auto notifyBufferResized(size_t newSize) -> void override;
  auto freeRegion(const BufferRegion& region) -> void override;
  auto reset() -> void override;

private:
  size_t currentOffset{};
  size_t maxBufferSize{};
  std::string name;
};

}
