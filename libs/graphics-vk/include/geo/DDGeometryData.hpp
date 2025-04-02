#pragma once

#include "dd/VertexList.hpp"

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

private:
  VertexList vertexList;
  std::vector<uint32_t> indexList;
};

}
