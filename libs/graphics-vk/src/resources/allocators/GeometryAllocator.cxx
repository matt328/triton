#include "GeometryAllocator.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "buffers/UploadPlan.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

GeometryAllocator::GeometryAllocator(std::shared_ptr<GeometryBufferPack> newGeometryBufferPack)
    : geometryBufferPack{std::move(newGeometryBufferPack)} {
}

auto GeometryAllocator::allocate(const GeometryData& data, TransferContext& transferContext)
    -> std::tuple<Handle<GeometryRegion>, std::vector<UploadData>> {
  auto geometryRegion = GeometryRegion{};
  auto uploadList = std::vector<UploadData>{};

  {
    auto indexDataSize = data.indexData.size() * sizeof(GpuIndexData);
    auto stagingRegion =
        transferContext.stagingAllocator->allocate(BufferRequest{.size = indexDataSize});
    geometryRegion.indexRegion = geometryBufferPack->getIndexBufferAllocator()
                                     .allocate(BufferRequest{.size = indexDataSize})
                                     .value();
    uploadList.push_back(UploadData{
        .dataSize = indexDataSize,
        .data = data.indexData.data(),
        .dstBuffer = geometryBufferPack->getIndexBuffer(),
        .stagingOffset = stagingRegion->offset,
        .dstOffset = geometryRegion.indexRegion.offset,
    });
  }

  {
    auto size = data.positionData.size() * sizeof(GpuVertexPositionData);
    auto stagingRegion = transferContext.stagingAllocator->allocate(BufferRequest{.size = size});
    geometryRegion.positionRegion = geometryBufferPack->getPositionBufferAllocator()
                                        .allocate(BufferRequest{.size = size})
                                        .value();
    uploadList.push_back(UploadData{
        .dataSize = size,
        .data = data.positionData.data(),
        .dstBuffer = geometryBufferPack->getPositionBuffer(),
        .stagingOffset = stagingRegion->offset,
        .dstOffset = geometryRegion.positionRegion.offset,
    });
  }

  // TODO(matt): finish allocating here.

  const auto handle = regionGenerator.requestHandle();
  regionTable.emplace(handle, geometryRegion);
  return {handle, uploadList};
}

auto GeometryAllocator::getRegionData(Handle<GeometryRegion> handle) const
    -> GpuGeometryRegionData {
  return {};
}

}
