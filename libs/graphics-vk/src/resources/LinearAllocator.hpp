#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class LinearAllocator : public IBufferAllocator {
public:
  explicit LinearAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~LinearAllocator() override = default;

  LinearAllocator(const LinearAllocator&) = delete;
  LinearAllocator(LinearAllocator&&) = delete;
  auto operator=(const LinearAllocator&) -> LinearAllocator& = delete;
  auto operator=(LinearAllocator&&) -> LinearAllocator& = delete;

  auto allocate(uint64_t resourceId, const UploadRequestVariant& req)
      -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> dstBuffer;
  size_t currentOffset{};
  size_t stagingCursor{};

  auto handleUploadGeometryRequest(size_t resourceId,
                                   size_t newOffset,
                                   size_t newStagingCursor,
                                   const UploadGeometryRequest& g)
      -> std::optional<MeshBufferRegion>;
};

}
