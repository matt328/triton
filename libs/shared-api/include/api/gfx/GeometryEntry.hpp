#pragma once

namespace tr {
constexpr uint32_t INVALID_OFFSET = std::numeric_limits<uint32_t>::max(); // 0xFFFFFFFF

/// Represents a single 'geometry' within the global geometry buffers
struct GeometryEntry {
  uint32_t vertexCount = 0;                 // Number of vertices this geometry has
  uint32_t positionOffset = 0;              // Offset into the position buffer
  uint32_t colorOffset = INVALID_OFFSET;    // Offset into the color buffer
  uint32_t texCoordOffset = INVALID_OFFSET; // Offset into the texCoordBuffer
  uint32_t normalOffset = INVALID_OFFSET;   // Offset into the normalBuffer

  uint32_t indexCount = 0;               // Number of indices
  uint32_t indexOffset = INVALID_OFFSET; // Offset into the index buffer
};
}
