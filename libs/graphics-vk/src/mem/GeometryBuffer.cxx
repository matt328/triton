#include "GeometryBuffer.hpp"
#include "mem/ArenaBuffer.hpp"

namespace tr {

GeometryBuffer::~GeometryBuffer() {
}

auto GeometryBuffer::addGeometry([[maybe_unused]] const DDGeometryData& geometryData)
    -> Handle<GeometryEntry> {
  return generator.requestHandle();
}

auto GeometryBuffer::evictGeometry([[maybe_unused]] Handle<GeometryEntry> handles) -> void {
}

[[nodiscard]] auto GeometryBuffer::getGeometryEntries() const -> std::vector<GeometryEntry> {
  return {};
}

}
