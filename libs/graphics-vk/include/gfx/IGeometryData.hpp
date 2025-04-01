#pragma once

#include "api/gfx/Topology.hpp"

namespace tr {

class IGeometryData {
public:
  IGeometryData() = default;
  virtual ~IGeometryData() = default;

  IGeometryData(const IGeometryData&) = default;
  IGeometryData(IGeometryData&&) = delete;
  auto operator=(const IGeometryData&) -> IGeometryData& = default;
  auto operator=(IGeometryData&&) -> IGeometryData& = delete;

  [[nodiscard]] virtual auto getVertexDataSize() const -> size_t = 0;
  [[nodiscard]] virtual auto getIndexDataSize() const -> size_t = 0;
  [[nodiscard]] virtual auto getVertexData() const -> const void* = 0;
  [[nodiscard]] virtual auto getIndexData() const -> const void* = 0;
  [[nodiscard]] virtual auto getIndexCount() const -> uint32_t = 0;
  [[nodiscard]] virtual auto getVertexCount() const -> uint32_t = 0;
  [[nodiscard]] virtual auto getTopology() const -> Topology {
    return Topology::Triangles;
  };
};

}
