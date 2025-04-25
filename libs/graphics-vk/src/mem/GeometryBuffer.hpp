#pragma once

#include "api/gfx/GeometryEntry.hpp"
#include "bk/HandleGenerator.hpp"

namespace tr {

class DDGeometryData;
class IBufferManager;
class ArenaBuffer;

class GeometryBuffer {
public:
  GeometryBuffer() = default;
  ~GeometryBuffer() = default;

  GeometryBuffer(const GeometryBuffer&) = delete;
  GeometryBuffer(GeometryBuffer&&) = delete;
  auto operator=(const GeometryBuffer&) -> GeometryBuffer& = delete;
  auto operator=(GeometryBuffer&&) -> GeometryBuffer& = delete;

  auto addGeometry(const DDGeometryData& geometryData) -> Handle<GeometryEntry>;
  auto evictGeometry(Handle<GeometryEntry> handles) -> void;

  [[nodiscard]] auto getGeometryEntries() const -> std::vector<GeometryEntry>;

private:
  IBufferManager* bufferManager{};

  std::unique_ptr<ArenaBuffer> positionBuffer;
  std::unique_ptr<ArenaBuffer> colorBuffer;
  std::unique_ptr<ArenaBuffer> texCoordBuffer;
  std::unique_ptr<ArenaBuffer> normalBuffer;
  std::unique_ptr<ArenaBuffer> indexBuffer;

  std::unique_ptr<ArenaBuffer> geometryEntryBuffer;

  HandleGenerator<GeometryEntry> generator;
  std::unordered_map<Handle<GeometryEntry>, GeometryEntry> entries;
};

}
