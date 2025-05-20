#pragma once

#include "api/gfx/GpuMaterialData.hpp"
#include "api/gw/GameObjectType.hpp"
#include "api/gw/TransformData.hpp"
#include "bk/Handle.hpp"

namespace tr {

struct StaticModelRequest {
  uint64_t requestId;
  std::string modelFilename;
  std::string entityName;
  std::optional<tr::TransformData> initialTransform = std::nullopt;
};

struct StaticModelLoaded {
  uint64_t requestId;
  std::string entityName;
  tr::GameObjectId objectId;
};

struct UploadGeometryRequest {
  uint64_t requestId;
  std::unique_ptr<GeometryData> data;
};

struct GeometryUploaded {
  uint64_t requestId;
  Handle<GpuGeometryRegionData> geometryHandle;
};

}
