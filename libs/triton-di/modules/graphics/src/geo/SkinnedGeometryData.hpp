#pragma once

#include "GeometryData.hpp"
#include "as/SkinnedVertex.hpp"

namespace tr {

class SkinnedGeometryData : public IGeometryData {
public:
  SkinnedGeometryData(std::vector<as::SkinnedVertex>&& newVertices,
                      std::vector<uint32_t>&& newIndices);
  ~SkinnedGeometryData() override = default;

  SkinnedGeometryData(const SkinnedGeometryData&) = delete;
  SkinnedGeometryData(SkinnedGeometryData&&) = delete;
  auto operator=(const SkinnedGeometryData&) -> SkinnedGeometryData& = delete;
  auto operator=(SkinnedGeometryData&&) -> SkinnedGeometryData& = delete;

  [[nodiscard]] auto getVertexDataSize() const -> size_t override;
  [[nodiscard]] auto getIndexDataSize() const -> size_t override;
  [[nodiscard]] auto getVertexData() const -> const void* override;
  [[nodiscard]] auto getIndexData() const -> const void* override;
  [[nodiscard]] auto getIndexCount() const -> size_t override;
  [[nodiscard]] auto getVertexCount() const -> size_t override;

private:
  std::vector<as::SkinnedVertex> vertices;
  std::vector<uint32_t> indices;
};

}
