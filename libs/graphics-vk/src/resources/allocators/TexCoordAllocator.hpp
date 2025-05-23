#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class TexCoordAllocator : public IBufferAllocator {
public:
  explicit TexCoordAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~TexCoordAllocator() override = default;

  TexCoordAllocator(const TexCoordAllocator&) = default;
  TexCoordAllocator(TexCoordAllocator&&) = delete;
  auto operator=(const TexCoordAllocator&) -> TexCoordAllocator& = default;
  auto operator=(TexCoordAllocator&&) -> TexCoordAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> positionBuffer;
  size_t insertOffset{};

  auto handleTexCoordDataRequest(size_t resourceId,
                                 size_t newOffset,
                                 size_t newStagingCursor,
                                 const TexCoordDataRequest& g) -> std::optional<MeshBufferRegion>;
};
}
