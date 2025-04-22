#pragma once

#include "bk/HandleGenerator.hpp"
namespace tr {

class DDGeometryData;
struct GeometryHandles;
class IBufferManager;
class ArenaBuffer;

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

class GeometryBuffer {
public:
  GeometryBuffer() = default;
  ~GeometryBuffer() = default;

  GeometryBuffer(const GeometryBuffer&) = delete;
  GeometryBuffer(GeometryBuffer&&) = delete;
  auto operator=(const GeometryBuffer&) -> GeometryBuffer& = delete;
  auto operator=(GeometryBuffer&&) -> GeometryBuffer& = delete;

  auto addGeometry(const DDGeometryData& geometryData) -> GeometryHandles;
  auto evictGeometry(GeometryHandles handles) -> void;

  [[nodiscard]] auto getGeometryEntries() const -> std::vector<GeometryEntry>;

private:
  IBufferManager* bufferManager{};

  std::unique_ptr<ArenaBuffer> positionBuffer;
  std::unique_ptr<ArenaBuffer> colorBuffer;
  std::unique_ptr<ArenaBuffer> texCoordBuffer;
  std::unique_ptr<ArenaBuffer> normalBuffer;
  std::unique_ptr<ArenaBuffer> indexBuffer;

  HandleGenerator<GeometryEntry> generator;
  std::unordered_map<Handle<GeometryEntry>, GeometryEntry> entries;
};

}
