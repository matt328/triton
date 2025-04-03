#pragma once

#include "api/gw/BoxCreateInfo.hpp"
#include "api/gfx/DDGeometryData.hpp"

namespace tr {

class GeometryGenerator {
public:
  GeometryGenerator() = default;
  ~GeometryGenerator() = default;

  GeometryGenerator(const GeometryGenerator&) = default;
  GeometryGenerator(GeometryGenerator&&) = delete;
  auto operator=(const GeometryGenerator&) -> GeometryGenerator& = default;
  auto operator=(GeometryGenerator&&) -> GeometryGenerator& = delete;

  auto generateBox(const BoxCreateInfo& info) -> DDGeometryData;
};

}
