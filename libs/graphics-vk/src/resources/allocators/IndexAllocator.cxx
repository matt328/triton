#include "IndexAllocator.hpp"

namespace tr {

IndexAllocator::IndexAllocator(Handle<ManagedBuffer> destinationBuffer)
    : indexBuffer{destinationBuffer} {
}

auto IndexAllocator::allocate(const UploadRequestData& requestData)
    -> std::optional<MeshBufferRegion> {
  auto dstOffset = insertOffset;
  auto dstStagingOffset = requestData.currentStagingOffset;

  auto visitor = [&](auto&& arg) -> std::optional<MeshBufferRegion> {
    using T = std::decay_t<decltype(arg)>;

    if constexpr (std::is_same_v<T, IndexDataRequest>) {
      return handleIndexDataRequest(requestData.resourceId, dstOffset, dstStagingOffset, arg);
    }
    Log.warn("IndexAllocator given a Request it couldn't handle");
    return std::nullopt;
  };

  return std::visit(visitor, requestData.requestVariant);
}

auto IndexAllocator::handleIndexDataRequest(size_t resourceId,
                                            size_t newOffset,
                                            size_t newStagingCursor,
                                            const IndexDataRequest& g)
    -> std::optional<MeshBufferRegion> {
  insertOffset += g.indexData.size();
  return std::make_optional<MeshBufferRegion>(
      MeshBufferRegion{.resourceId = resourceId,
                       .regions = {CopyRegion{.stagingOffset = newStagingCursor,
                                              .destinationOffset = newOffset,
                                              .size = g.indexData.size(),
                                              .dstBuffer = indexBuffer}}});
}

auto IndexAllocator::reset() -> void {
  insertOffset = 0;
}

}
