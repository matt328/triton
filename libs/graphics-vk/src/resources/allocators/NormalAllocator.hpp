#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class NormalAllocator : public IBufferAllocator {
public:
  explicit NormalAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~NormalAllocator() override = default;

  NormalAllocator(const NormalAllocator&) = default;
  NormalAllocator(NormalAllocator&&) = delete;
  auto operator=(const NormalAllocator&) -> NormalAllocator& = default;
  auto operator=(NormalAllocator&&) -> NormalAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> positionBuffer;
  size_t insertOffset{};

  auto handleNormalDataRequest(size_t resourceId,
                               size_t newOffset,
                               size_t newStagingCursor,
                               const NormalDataRequest& g) -> std::optional<MeshBufferRegion>;
};
}
