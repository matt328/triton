#include "GeometryDispatcher.hpp"
#include "api/fx/ResourceEvents.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

GeometryDispatcher::GeometryDispatcher(std::shared_ptr<GeometryBufferPack> newGeometryBufferPack)
    : bufferPack{std::move(newGeometryBufferPack)} {
}

auto GeometryDispatcher::processUploadRequest(const UploadGeometryRequest& request)
    -> std::vector<CopyRegion> {

  auto copyRegions = std::vector<CopyRegion>{};

  if (!request.data->indexData.empty()) {
    auto size = request.data->indexData.size() * sizeof(GpuIndexData);
    auto indexPosition = indexAllocator->allocate(BufferRequest{.size = size});

    if (indexPosition) {
      copyRegions.push_back(CopyRegion{.destinationOffset = indexPosition->offset,
                                       .size = size,
                                       .srcData = request.data->indexData.data(),
                                       .dstBuffer = bufferPack->getIndexBuffer()});
    }
  }

  return copyRegions;
}

}
