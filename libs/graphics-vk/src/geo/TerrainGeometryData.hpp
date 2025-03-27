#pragma once

#include "gfx/IGeometryData.hpp"
#include "as/TerrainVertex.hpp"

namespace tr {

class TerrainGeometryData : public IGeometryData {
public:
  TerrainGeometryData(std::vector<as::TerrainVertex>&& newVertices,
                      std::vector<uint32_t>&& newIndices);
  ~TerrainGeometryData() override = default;

  TerrainGeometryData(const TerrainGeometryData&) = delete;
  TerrainGeometryData(TerrainGeometryData&&) = delete;
  auto operator=(const TerrainGeometryData&) -> TerrainGeometryData& = delete;
  auto operator=(TerrainGeometryData&&) -> TerrainGeometryData& = delete;

  [[nodiscard]] auto getVertexDataSize() const -> size_t override;
  [[nodiscard]] auto getIndexDataSize() const -> size_t override;
  [[nodiscard]] auto getVertexData() const -> const void* override;
  [[nodiscard]] auto getIndexData() const -> const void* override;
  [[nodiscard]] auto getIndexCount() const -> uint32_t override;
  [[nodiscard]] auto getVertexCount() const -> uint32_t override;

private:
  std::vector<as::TerrainVertex> vertices;
  std::vector<uint32_t> indices;
};

}
