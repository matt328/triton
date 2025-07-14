#pragma once

#include "api/gfx/GeometryData.hpp"
#include "as/Model.hpp"
#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

/// Used to partition a flat list of these into a vector<SubBatch> such that each SubBatch contains
/// a vector<StagingRequirements>
struct StagingRequirements {
  uint64_t requestId;
  std::type_index responseType;
  std::optional<size_t> geometryBytes = std::nullopt;
  std::optional<size_t> imageBytes = std::nullopt;
  std::shared_ptr<GeometryData> geometryData;
  std::vector<std::shared_ptr<as::ImageData>> imageDataList;
};

struct SubBatch {
  uint8_t priority;
  std::vector<StagingRequirements> items;
};

struct BufferUploadItem {
  uint64_t requestId{};
  std::type_index responseType;

  size_t dataSize{};
  std::shared_ptr<std::vector<std::byte>> data = nullptr;
  Handle<ManagedBuffer> dstBuffer{};

  size_t stagingOffset{};
  size_t dstOffset{};
};

struct ImageUploadItem {
  uint64_t requestId{};
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
  std::vector<BufferUploadItem> bufferUploadItems;
  std::vector<ImageUploadItem> imageUploadItems;
};

struct BufferUploadResult {
  uint64_t requestId{};
  std::type_index responseType;
};

struct ImageUploadResult {
  uint64_t requestId{};
  std::type_index responseType;
};

struct UploadSubBatchResult {
  std::vector<BufferUploadResult> bufferResults;
  std::vector<ImageUploadResult> imageResults;
};

}
