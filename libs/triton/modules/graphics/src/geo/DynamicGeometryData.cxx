#include "DynamicGeometryData.hpp"
#include "as/DynamicVertex.hpp"

namespace tr {

DynamicGeometryData::DynamicGeometryData(std::vector<as::DynamicVertex>&& newVertices,
                                         std::vector<uint32_t>&& newIndices)
    : vertices(std::move(newVertices)), indices(std::move(newIndices)) {
}

auto DynamicGeometryData::getVertexDataSize() const -> size_t {
  return sizeof(as::DynamicVertex) * vertices.size();
}

auto DynamicGeometryData::getIndexDataSize() const -> size_t {
  return sizeof(uint32_t) * indices.size();
}

auto DynamicGeometryData::getVertexData() const -> const void* {
  return static_cast<const void*>(vertices.data());
}

auto DynamicGeometryData::getIndexData() const -> const void* {
  return static_cast<const void*>(indices.data());
}

auto DynamicGeometryData::getIndexCount() const -> size_t {
  return indices.size();
}

auto DynamicGeometryData::getVertexCount() const -> size_t {
  return vertices.size();
}

}
