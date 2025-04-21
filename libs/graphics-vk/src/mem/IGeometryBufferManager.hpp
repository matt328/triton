#pragma once

#include "as/VertexList.hpp"
#include "bk/Handle.hpp"
#include "mem/BufferWrapper.hpp"

namespace tr {

struct GeometryUploadRequest {
  VertexFormat format;             // Enum or descriptor of vertex layout
  std::vector<uint8_t> vertexData; // Raw vertex data (layout-specific)
  std::vector<uint32_t> indices;   // Index data (optional, can be empty)
  uint32_t vertexCount;
};

struct GeometryUploadResult {
  uint32_t geometryRegionId; // Index into GpuGeometryRegionData
};

class IGeometryBufferManager {
public:
  IGeometryBufferManager() = default;
  virtual ~IGeometryBufferManager() = default;

  IGeometryBufferManager(const IGeometryBufferManager&) = default;
  IGeometryBufferManager(IGeometryBufferManager&&) = delete;
  auto operator=(const IGeometryBufferManager&) -> IGeometryBufferManager& = default;
  auto operator=(IGeometryBufferManager&&) -> IGeometryBufferManager& = delete;

  // Uploads new geometry and returns its region ID
  virtual auto uploadGeometry(const GeometryUploadRequest& request) -> GeometryUploadResult = 0;

  // GPU buffers to bind to shaders
  [[nodiscard]] auto getVertexAttributeBuffer(VertexAttribute attr) const -> Handle<BufferWrapper>;
  [[nodiscard]] auto getIndexBuffer() const -> Handle<BufferWrapper>;
  [[nodiscard]] auto getGeometryRegionBuffer() const -> Handle<BufferWrapper>;

  // Resets temporary regions (if desired between frames, for dynamic meshes)
  void resetFrame();
};

}
