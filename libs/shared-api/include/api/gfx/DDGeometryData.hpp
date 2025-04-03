#pragma once

#include "as/VertexList.hpp"

namespace tr {

class DDGeometryData {
public:
  ~DDGeometryData() = default;

  DDGeometryData(VertexList&& vList, std::vector<uint32_t>&& iList) noexcept
      : vertexList(std::move(vList)), indexList(std::move(iList)) {
  }

  DDGeometryData(const DDGeometryData&) = delete;
  DDGeometryData(DDGeometryData&&) = default;
  auto operator=(const DDGeometryData&) -> DDGeometryData& = delete;
  auto operator=(DDGeometryData&&) -> DDGeometryData& = default;

  [[nodiscard]] auto getVertexList() const -> const VertexList& {
    return vertexList;
  }

  [[nodiscard]] auto getIndexList() const -> const std::vector<uint32_t>& {
    return indexList;
  }

private:
  VertexList vertexList;
  std::vector<uint32_t> indexList;
};

}
