#pragma once

#include "buffers/ManagedBuffer.hpp"
#include "mem/BufferRegion.hpp"

namespace tr {

struct BufferRequest {
  size_t size;
};

struct ResizeRequest {
  Handle<ManagedBuffer> bufferHandle;
  size_t newSize;
};

class IBufferAllocator {
public:
  explicit IBufferAllocator(Handle<ManagedBuffer> handle) : bufferHandle{handle} {
  }
  virtual ~IBufferAllocator() = default;

  IBufferAllocator(const IBufferAllocator&) = default;
  IBufferAllocator(IBufferAllocator&&) = delete;
  auto operator=(const IBufferAllocator&) -> IBufferAllocator& = default;
  auto operator=(IBufferAllocator&&) -> IBufferAllocator& = delete;

  virtual auto allocate(const BufferRequest& requestData) -> BufferRegion = 0;
  virtual auto checkSize(const BufferRequest& requestData) -> std::optional<ResizeRequest> = 0;
  virtual auto notifyBufferResized(size_t newSize) -> void = 0;
  virtual auto freeRegion(const BufferRegion& region) -> void = 0;
  virtual auto reset() -> void = 0;

protected:
  Handle<ManagedBuffer> bufferHandle;
};

}
