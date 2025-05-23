#pragma once

#include "bk/Handle.hpp"
#include "resources/UploadRequestVariant.hpp"

namespace tr {

struct ManagedBuffer;

struct UploadRequestData {
  uint64_t resourceId;
  size_t currentStagingOffset;
  UploadRequestVariant requestVariant;
};

struct CopyRegion {
  size_t stagingOffset;
  size_t destinationOffset;
  size_t size;
  Handle<ManagedBuffer> dstBuffer;
};

struct MeshBufferRegion {
  uint64_t resourceId;
  std::vector<CopyRegion> regions;
};

class IBufferAllocator {
public:
  IBufferAllocator() = default;
  virtual ~IBufferAllocator() = default;

  IBufferAllocator(const IBufferAllocator&) = default;
  IBufferAllocator(IBufferAllocator&&) = delete;
  auto operator=(const IBufferAllocator&) -> IBufferAllocator& = default;
  auto operator=(IBufferAllocator&&) -> IBufferAllocator& = delete;

  virtual auto allocate(const UploadRequestData& requestData)
      -> std::optional<MeshBufferRegion> = 0;
  virtual auto reset() -> void = 0;
};

}
