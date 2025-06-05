#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {

struct UploadData {
  size_t dataSize{};
  std::shared_ptr<void> data = nullptr;
  Handle<ManagedBuffer> dstBuffer{};

  size_t stagingOffset{};
  size_t dstOffset{};
};

struct UploadPlan {
  std::vector<UploadData> uploads{};
  size_t stagingSize{};
  Handle<ManagedBuffer> stagingBuffer{};

  auto sortByBuffer() -> void {
    std::ranges::sort(uploads, [](const UploadData& a, const UploadData& b) {
      return a.dstBuffer.id < b.dstBuffer.id;
    });
  }
};

}
