#pragma once

#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

struct ImageUploadData {
  std::shared_ptr<std::vector<std::byte>> data = nullptr;
  size_t dataSize{}; // size in bytes

  Handle<ManagedImage> dstImage;
  vk::ImageSubresourceLayers subresource{};
  vk::Offset3D imageOffset{.x = 0, .y = 0, .z = 0};
  vk::Extent3D imageExtent{};

  size_t stagingBufferOffset{};
};

struct ImageUploadPlan {
  std::vector<ImageUploadData> uploads{};
  size_t stagingSize{};
  Handle<ManagedBuffer> stagingBuffer;
};

}
