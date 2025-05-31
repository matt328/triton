#pragma once

#include "api/gfx/Geometry.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "api/gw/GameObjectType.hpp"
#include "api/gw/TransformData.hpp"
#include "as/Model.hpp"
#include "bk/Handle.hpp"

namespace tr {

struct BeginResourceBatch {
  uint64_t batchId;
};

struct EndResourceBatch {
  uint64_t batchId;
};

/// Clients emits
/// AssetSystem handles
struct StaticModelRequest {
  uint64_t batchId;
  uint64_t requestId;
  std::string modelFilename;
  std::string entityName;
  std::optional<tr::TransformData> initialTransform = std::nullopt;
};

/// AssetSystem emits
/// EntityManager handles
struct StaticModelUploaded {
  uint64_t batchId;
  uint64_t requestId;
  std::string entityName;
  Handle<Geometry> geometryHandle;
};

/// EntityManager emits
/// Client Handles
struct StaticModelResponse {
  uint64_t batchId;
  uint64_t requestId;
  std::string entityName;
  GameObjectId gameObjectId;
};

struct DynamicModelRequest {
  uint64_t batchId;
  uint64_t requestId;
  std::string modelFilename;
  std::string entityName;
  std::optional<tr::TransformData> initialTransform = std::nullopt;
};

struct DynamicModelResponse {
  uint64_t batchId;
  uint64_t requestId;
  std::string entityName;
  tr::GameObjectId objectId;
};

struct UploadGeometryRequest {
  uint64_t batchId;
  uint64_t requestId;
  std::unique_ptr<GeometryData> data;
};

struct UploadGeometryResponse {
  uint64_t batchId;
  uint64_t requestId;
  Handle<GpuGeometryRegionData> geometryHandle;
};

struct UploadImageRequest {
  uint64_t batchId;
  uint64_t requestId;
  std::unique_ptr<as::ImageData> data;
};

struct UploadImageResponse {
  uint64_t batchId;
  uint64_t requestId;
  // Handle<ManagedImage> imageHandle;
};

/*
  ManagedImage and GpuGeometryRegionData will need abstractions so they can be returned to the
  client
*/

}
