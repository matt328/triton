#include "LinearAllocator.hpp"
#include "bk/Variants.hpp"

namespace tr {

/*
  Create GeometryAllocator, this one just contains a separate IBufferAllocator for each Geometry
  Attribute Buffer. It also aggregates the logic to make sure GeometryRequests are processed all or
  nothing It will split the UploadGeometryRequest into separate AttributeRequests and delegate them
  to the appropriate child IBufferAllocators
*/

LinearAllocator::LinearAllocator(Handle<ManagedBuffer> destinationBuffer)
    : dstBuffer{destinationBuffer} {
}

auto LinearAllocator::allocate(uint64_t resourceId, const UploadRequestVariant& req)
    -> std::optional<MeshBufferRegion> {

  auto dstOffset = currentOffset;
  auto dstStagingCursor = stagingCursor;

  auto region = match(
      req,
      [&](const UploadGeometryRequest& g) {
        return handleUploadGeometryRequest(resourceId, dstOffset, dstStagingCursor, g);
      },
      []([[maybe_unused]] auto&& unhandled) { return std::nullopt; });

  return region;
}

auto LinearAllocator::reset() -> void {
  currentOffset = 0;
  stagingCursor = 0;
}

auto LinearAllocator::handleUploadGeometryRequest(size_t resourceId,
                                                  size_t newOffset,
                                                  size_t newStagingCursor,
                                                  const UploadGeometryRequest& g)
    -> std::optional<MeshBufferRegion> {
  currentOffset += g.data->positionData.size();
  stagingCursor += g.data->positionData.size();
  return std::make_optional<MeshBufferRegion>({.resourceId = resourceId,
                                               .regions = {{.stagingOffset = newStagingCursor,
                                                            .destinationOffset = newOffset,
                                                            .size = g.data->positionData.size(),
                                                            .dstBuffer = dstBuffer}}});
}

}
