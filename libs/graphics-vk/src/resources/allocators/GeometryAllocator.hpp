#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "mem/BufferRegion.hpp"
#include "resources/TransferContext.hpp"

namespace tr {

struct GeometryData;
struct GpuGeometryRegionData;
struct UploadData;
class GeometryBufferPack;

struct GeometryRegion {
  uint32_t indexCount{};
  BufferRegion indexRegion;
  BufferRegion positionRegion;
  std::optional<BufferRegion> normalRegion;
  std::optional<BufferRegion> texCoordRegion;
  std::optional<BufferRegion> colorRegion;
  std::optional<BufferRegion> animationDataRegion;
};

struct BufferAllocation {
  size_t dataSize{};
  std::shared_ptr<std::vector<std::byte>> data = nullptr;
  Handle<ManagedBuffer> dstBuffer{};
  size_t stagingOffset{};
  size_t dstOffset{};
};

struct GeometryAllocation {
  Handle<GeometryRegion> regionHandle;
  std::vector<BufferAllocation> bufferAllocations;
};

class GeometryAllocator {
public:
  explicit GeometryAllocator(std::shared_ptr<GeometryBufferPack> newGeometryBufferPack);
  ~GeometryAllocator() = default;

  GeometryAllocator(const GeometryAllocator&) = delete;
  GeometryAllocator(GeometryAllocator&&) = delete;
  auto operator=(const GeometryAllocator&) -> GeometryAllocator& = delete;
  auto operator=(GeometryAllocator&&) -> GeometryAllocator& = delete;

  auto allocate(const GeometryData& data, TransferContext& transferContext) -> GeometryAllocation;
  [[nodiscard]] auto getRegionData(Handle<GeometryRegion> handle) const -> GpuGeometryRegionData;

private:
  std::shared_ptr<GeometryBufferPack> geometryBufferPack;
  HandleGenerator<GeometryRegion> regionGenerator{};
  std::unordered_map<Handle<GeometryRegion>, GeometryRegion> regionTable;
};

}
