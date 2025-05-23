#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

struct GeometryBuffers {
  Handle<ManagedBuffer> indexBuffer;
  Handle<ManagedBuffer> positionBuffer;
  Handle<ManagedBuffer> colorBuffer;
  Handle<ManagedBuffer> texCoordBuffer;
  Handle<ManagedBuffer> normalBuffer;
  Handle<ManagedBuffer> animationBuffer;
};

class GeometryAllocator : public IBufferAllocator {
public:
  explicit GeometryAllocator(const GeometryBuffers& geometryBuffers);
  ~GeometryAllocator() override = default;

  GeometryAllocator(const GeometryAllocator&) = delete;
  GeometryAllocator(GeometryAllocator&&) = delete;
  auto operator=(const GeometryAllocator&) -> GeometryAllocator& = delete;
  auto operator=(GeometryAllocator&&) -> GeometryAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  std::unique_ptr<IBufferAllocator> indexAllocator;
  std::unique_ptr<IBufferAllocator> positionAllocator;
  std::unique_ptr<IBufferAllocator> colorAllocator;
  std::unique_ptr<IBufferAllocator> texCoordAllocator;
  std::unique_ptr<IBufferAllocator> normalAllocator;
  std::unique_ptr<IBufferAllocator> animationAllocator;

  auto handleUploadGeometryRequest(size_t resourceId,
                                   size_t stagingOffset,
                                   const UploadGeometryRequest& g)
      -> std::optional<MeshBufferRegion>;
};

}
