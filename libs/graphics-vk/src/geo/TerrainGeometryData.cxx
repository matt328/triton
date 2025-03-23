#include "TerrainGeometryData.hpp"
#include "as/TerrainVertex.hpp"

namespace tr {

TerrainGeometryData::TerrainGeometryData(std::vector<as::TerrainVertex>&& newVertices,
                                         std::vector<uint32_t>&& newIndices)
    : vertices(std::move(newVertices)), indices(std::move(newIndices)) {
}

auto TerrainGeometryData::getVertexDataSize() const -> size_t {
  return sizeof(as::TerrainVertex) * vertices.size();
}

auto TerrainGeometryData::getIndexDataSize() const -> size_t {
  return sizeof(uint32_t) * indices.size();
}

auto TerrainGeometryData::getVertexData() const -> const void* {
  return static_cast<const void*>(vertices.data());
}

auto TerrainGeometryData::getIndexData() const -> const void* {
  return static_cast<const void*>(indices.data());
}

auto TerrainGeometryData::getIndexCount() const -> uint32_t {
  return indices.size();
}

auto TerrainGeometryData::getVertexCount() const -> uint32_t {
  return vertices.size();
}

}
