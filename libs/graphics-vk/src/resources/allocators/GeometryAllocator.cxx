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
    auto size = data.indexData->size();
    auto stagingRegion = transferContext.stagingAllocator->allocate(BufferRequest{.size = size});
    Log.trace("Allocated index data in staging buffer, region.size={}, region.offset={}",
              stagingRegion->size,
              stagingRegion->offset);
    geometryRegion.indexRegion =
        geometryBufferPack->getIndexBufferAllocator().allocate(BufferRequest{.size = size}).value();

    geometryRegion.indexCount = data.indexData->size() / sizeof(GpuIndexData);

    uploadList.push_back(UploadData{
        .dataSize = size,
        .data = data.indexData,
        .dstBuffer = geometryBufferPack->getIndexBuffer(),
        .stagingOffset = stagingRegion->offset,
        .dstOffset = geometryRegion.indexRegion.offset,
    });
  }

  {
    auto size = data.positionData->size();
    auto stagingRegion = transferContext.stagingAllocator->allocate(BufferRequest{.size = size});
    Log.trace("Allocated position data in staging buffer, region.size={}, region.offset={}",
              stagingRegion->size,
              stagingRegion->offset);
    geometryRegion.positionRegion = geometryBufferPack->getPositionBufferAllocator()
                                        .allocate(BufferRequest{.size = size})
                                        .value();
    uploadList.push_back(UploadData{
        .dataSize = size,
        .data = data.positionData,
        .dstBuffer = geometryBufferPack->getPositionBuffer(),
        .stagingOffset = stagingRegion->offset,
        .dstOffset = geometryRegion.positionRegion.offset,
    });
  }

  if (data.texCoordData != nullptr) {
    auto size = data.texCoordData->size();
    auto stagingRegion = transferContext.stagingAllocator->allocate(BufferRequest{.size = size});
    Log.trace("Allocated texCoord data in staging buffer, region.size={}, region.offset={}",
              stagingRegion->size,
              stagingRegion->offset);
    geometryRegion.texCoordRegion = geometryBufferPack->getTexCoordBufferAllocator()
                                        .allocate(BufferRequest{.size = size})
                                        .value();
    uploadList.push_back(UploadData{
        .dataSize = size,
        .data = data.texCoordData,
        .dstBuffer = geometryBufferPack->getTexCoordBuffer(),
        .stagingOffset = stagingRegion->offset,
        .dstOffset = geometryRegion.texCoordRegion->offset,
    });
  }

  const auto handle = regionGenerator.requestHandle();
  regionTable.emplace(handle, geometryRegion);
  return {handle, uploadList};
}

auto GeometryAllocator::getRegionData(Handle<GeometryRegion> handle) const
    -> GpuGeometryRegionData {
  assert(regionTable.contains(handle) && "No RegionTable entry for given handle");
  const auto& region = regionTable.at(handle);

  auto regionData = GpuGeometryRegionData{
      .indexCount = region.indexCount,
      .indexOffset = static_cast<uint32_t>(region.indexRegion.offset / sizeof(GpuIndexData)),
      .positionOffset = static_cast<uint32_t>(region.positionRegion.offset)};
  if (region.texCoordRegion) {
    regionData.texCoordOffset = region.texCoordRegion->offset;
  }
  if (region.normalRegion) {
    regionData.normalOffset = region.normalRegion->offset;
  }

  return regionData;
}

}
