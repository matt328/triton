#pragma once

namespace tr {

enum class BufferAlias : uint8_t {
  IndirectCommand = 0,
  IndirectCommandCount,
  GeometryRegion,
  Count
};

enum class ImageAlias : uint8_t {
  GeometryColorImage = 0,
  SwapchainImage,
  DepthImage,
  Count
};

}
