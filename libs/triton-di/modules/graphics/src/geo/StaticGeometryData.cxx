#include "StaticGeometryData.hpp"
#include "as/StaticVertex.hpp"

namespace tr {

StaticGeometryData::StaticGeometryData(std::vector<as::StaticVertex>&& newVertices,
                                       std::vector<uint32_t>&& newIndices)
    : vertices(std::move(newVertices)), indices(std::move(newIndices)) {
}

auto StaticGeometryData::getVertexDataSize() const -> size_t {
  return sizeof(vertices[0]) * vertices.size();
}

auto StaticGeometryData::getIndexDataSize() const -> size_t {
  return sizeof(indices[0]) * indices.size();
}

auto StaticGeometryData::getVertexData() const -> const void* {
  return static_cast<const void*>(vertices.data());
}

auto StaticGeometryData::getIndexData() const -> const void* {
  return static_cast<const void*>(indices.data());
}

auto StaticGeometryData::getIndexCount() const -> size_t {
  return indices.size();
}

auto StaticGeometryData::getVertexCount() const -> size_t {
  return indices.size();
}

}
