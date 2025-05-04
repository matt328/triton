#pragma once

namespace tr {

enum class BufferSizeType : uint8_t {
  Fixed = 0,
  Arena,
};

struct BufferRequest {
  BufferSizeType sizeType;
  vk::BufferCreateInfo bufferCreateInfo;
  vma::AllocationCreateInfo allocationCreateInfo;
  std::optional<size_t> itemStride;
};

}
