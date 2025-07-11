#pragma once

#include "api/fx/ResourceEvents.hpp"
#include "api/gfx/Geometry.hpp"
#include "api/gfx/GeometryData.hpp"
#include "buffers/ManagedBuffer.hpp"

namespace tr {

struct UploadData {
  size_t dataSize{};
  std::shared_ptr<std::vector<std::byte>> data = nullptr;
  Handle<ManagedBuffer> dstBuffer{};

  size_t stagingOffset{};
  size_t dstOffset{};
};

struct UploadPlan {
  std::unordered_map<uint64_t, std::vector<UploadData>> uploadsByRequest{};
  std::unordered_map<uint64_t, Handle<Geometry>> geometryDataByRequest{};
  size_t stagingSize{};
  Handle<ManagedBuffer> stagingBuffer{};

  auto getSortedUploads() const -> std::vector<UploadData> {
    std::vector<UploadData> result;
    for (const auto& [_, uploads] : uploadsByRequest) {
      result.insert(result.end(), uploads.begin(), uploads.end());
    }
    std::ranges::sort(result, [](const UploadData& a, const UploadData& b) {
      return a.dstBuffer.id < b.dstBuffer.id;
    });
    return result;
  }

  struct ResponseEventVisitor {
    Handle<Geometry> geometryHandle;

    template <typename T>
    void operator()(T& arg) const {
      if constexpr (std::is_same_v<T, StaticModelUploaded>) {
        arg.geometryHandle = geometryHandle;
      }
    }
  };
};

}
