#pragma once

#include "api/gfx/Geometry.hpp"
#include "api/gfx/GeometryData.hpp"
#include "as/Model.hpp"
#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "img/ManagedImage.hpp"
#include "resources/allocators/GeometryAllocator.hpp"

namespace tr {

struct Cargo {
  uint64_t batchId;
  uint64_t requestId;
  std::string entityName;
};

/// Used to partition a flat list of these into a vector<SubBatch> such that each SubBatch contains
/// a vector<StagingRequirements>
struct StagingRequirements {
  Cargo cargo;
  std::type_index responseType;
  std::optional<size_t> geometrySize = std::nullopt;
  std::optional<size_t> imageSize = std::nullopt;
  std::shared_ptr<GeometryData> geometryData;
  std::vector<std::shared_ptr<as::ImageData>> imageDataList;
};

struct SubBatch {
  uint8_t priority;
  std::vector<StagingRequirements> items;
};

struct GeometryUpload {
  Cargo cargo;
  std::type_index responseType;
  GeometryAllocation bufferAllocation;
};

struct ImageUploadItem {
  Cargo cargo;
  std::type_index responseType;

  std::shared_ptr<std::vector<std::byte>> data = nullptr;
  size_t dataSize{};

  Handle<ManagedImage> dstImage;
  vk::ImageSubresourceLayers subresource{};
  vk::Offset3D imageOffset{.x = 0, .y = 0, .z = 0};
  vk::Extent3D imageExtent{};

  size_t stagingBufferOffset{};
};

struct UploadSubBatch {
  std::vector<GeometryUpload> bufferUploadItems;
  std::vector<ImageUploadItem> imageUploadItems;
};

struct SubBatchResult {
  Cargo cargo;
  std::type_index responseType;
  Handle<Geometry> geometryHandle;
  Handle<TextureTag> textureHandle;
};

struct UploadSubBatchResult {
  std::unordered_map<uint64_t, std::vector<SubBatchResult>> resultsByRequest;
};

}
