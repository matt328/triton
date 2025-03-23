#pragma once

#include "cm/IGeometryData.hpp"
#include "as/DynamicVertex.hpp"

namespace tr {

class DynamicGeometryData : public IGeometryData {
public:
  DynamicGeometryData(std::vector<as::DynamicVertex>&& newVertices,
                      std::vector<uint32_t>&& newIndices);
  ~DynamicGeometryData() override = default;

  DynamicGeometryData(const DynamicGeometryData&) = delete;
  DynamicGeometryData(DynamicGeometryData&&) = delete;
  auto operator=(const DynamicGeometryData&) -> DynamicGeometryData& = delete;
  auto operator=(DynamicGeometryData&&) -> DynamicGeometryData& = delete;

  [[nodiscard]] auto getVertexDataSize() const -> size_t override;
  [[nodiscard]] auto getIndexDataSize() const -> size_t override;
  [[nodiscard]] auto getVertexData() const -> const void* override;
  [[nodiscard]] auto getIndexData() const -> const void* override;
  [[nodiscard]] auto getIndexCount() const -> uint32_t override;
  [[nodiscard]] auto getVertexCount() const -> uint32_t override;

private:
  std::vector<as::DynamicVertex> vertices;
  std::vector<uint32_t> indices;
};

}
