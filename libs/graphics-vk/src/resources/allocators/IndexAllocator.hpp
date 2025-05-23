#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class IndexAllocator : public IBufferAllocator {
public:
  explicit IndexAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~IndexAllocator() override = default;

  IndexAllocator(const IndexAllocator&) = default;
  IndexAllocator(IndexAllocator&&) = delete;
  auto operator=(const IndexAllocator&) -> IndexAllocator& = default;
  auto operator=(IndexAllocator&&) -> IndexAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> indexBuffer;
  size_t insertOffset{};

  auto handleIndexDataRequest(size_t resourceId,
                              size_t newOffset,
                              size_t newStagingCursor,
                              const IndexDataRequest& g) -> std::optional<MeshBufferRegion>;
};

}
