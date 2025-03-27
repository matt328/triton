#pragma once

#include "gfx/IGeometryData.hpp"
#include "as/StaticVertex.hpp"

namespace tr {

class StaticGeometryData : public IGeometryData {
public:
  StaticGeometryData(const std::vector<as::StaticVertex>&& newVertices,
                     const std::vector<uint32_t>&& newIndices);
  ~StaticGeometryData() override = default;

  StaticGeometryData(const StaticGeometryData&) = delete;
  StaticGeometryData(StaticGeometryData&&) = delete;
  auto operator=(const StaticGeometryData&) -> StaticGeometryData& = delete;
  auto operator=(StaticGeometryData&&) -> StaticGeometryData& = delete;

  [[nodiscard]] auto getVertexDataSize() const -> size_t override;
  [[nodiscard]] auto getIndexDataSize() const -> size_t override;
  [[nodiscard]] auto getVertexData() const -> const void* override;
  [[nodiscard]] auto getIndexData() const -> const void* override;
  [[nodiscard]] auto getIndexCount() const -> uint32_t override;
  [[nodiscard]] auto getVertexCount() const -> uint32_t override;

private:
  std::vector<as::StaticVertex> vertices;
  std::vector<uint32_t> indices;
};

}
