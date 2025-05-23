#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class PositionAllocator : public IBufferAllocator {
public:
  explicit PositionAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~PositionAllocator() override = default;

  PositionAllocator(const PositionAllocator&) = default;
  PositionAllocator(PositionAllocator&&) = delete;
  auto operator=(const PositionAllocator&) -> PositionAllocator& = default;
  auto operator=(PositionAllocator&&) -> PositionAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> positionBuffer;
  size_t insertOffset{};

  auto handlePositionDataRequest(size_t resourceId,
                                 size_t newOffset,
                                 size_t newStagingCursor,
                                 const PositionDataRequest& g) -> std::optional<MeshBufferRegion>;
};
}
