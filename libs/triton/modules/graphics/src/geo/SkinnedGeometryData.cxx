#include "SkinnedGeometryData.hpp"
#include "as/SkinnedVertex.hpp"

namespace tr {

SkinnedGeometryData::SkinnedGeometryData(std::vector<as::SkinnedVertex>&& newVertices,
                                         std::vector<uint32_t>&& newIndices)
    : vertices(std::move(newVertices)), indices(std::move(newIndices)) {
}

auto SkinnedGeometryData::getVertexDataSize() const -> size_t {
  return sizeof(as::SkinnedVertex) * vertices.size();
}

auto SkinnedGeometryData::getIndexDataSize() const -> size_t {
  return sizeof(uint32_t) * indices.size();
}

auto SkinnedGeometryData::getVertexData() const -> const void* {
  return static_cast<const void*>(vertices.data());
}

auto SkinnedGeometryData::getIndexData() const -> const void* {
  return static_cast<const void*>(indices.data());
}

auto SkinnedGeometryData::getIndexCount() const -> size_t {
  return indices.size();
}

auto SkinnedGeometryData::getVertexCount() const -> size_t {
  return indices.size();
}

}
