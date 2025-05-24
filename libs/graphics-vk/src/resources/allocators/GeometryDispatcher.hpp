#pragma once

#include "resources/allocators/CopyRegion.hpp"

namespace tr {

struct UploadGeometryRequest;
class IBufferAllocator;
class GeometryBufferPack;

class GeometryDispatcher {
public:
  explicit GeometryDispatcher(std::shared_ptr<GeometryBufferPack> newGeometryBufferPack);
  ~GeometryDispatcher() = default;

  GeometryDispatcher(const GeometryDispatcher&) = delete;
  GeometryDispatcher(GeometryDispatcher&&) = delete;
  auto operator=(const GeometryDispatcher&) -> GeometryDispatcher& = delete;
  auto operator=(GeometryDispatcher&&) -> GeometryDispatcher& = delete;

  auto processUploadRequest(const UploadGeometryRequest& request) -> std::vector<CopyRegion>;

private:
  std::shared_ptr<GeometryBufferPack> bufferPack;
  std::unique_ptr<IBufferAllocator> indexAllocator;
  std::unique_ptr<IBufferAllocator> positionAllocator;
  std::unique_ptr<IBufferAllocator> colorAllocator;
  std::unique_ptr<IBufferAllocator> texCoordAllocator;
  std::unique_ptr<IBufferAllocator> normalAllocator;
  std::unique_ptr<IBufferAllocator> animationAllocator;
};

}
