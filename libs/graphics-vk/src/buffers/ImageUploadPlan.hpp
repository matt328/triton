#pragma once

#include "api/fx/ResourceEvents.hpp"
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
  uint64_t requestId{};
};

struct ImageUploadPlan {
  std::unordered_map<uint64_t, std::vector<ImageUploadData>> uploadsByRequest{};
  size_t stagingSize{};
  Handle<ManagedBuffer> stagingBuffer;

  struct ResponseEventVisitor {
    Handle<TextureTag> textureHandle;

    template <typename T>
    void operator()(T& arg) const {
      if constexpr (std::is_same_v<T, StaticModelUploaded>) {
        arg.textureHandle.emplace(textureHandle);
      }
    }
  };
};

}
