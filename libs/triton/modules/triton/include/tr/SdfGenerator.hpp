#pragma once

#include "cm/Rando.hpp"
#include "cm/SdfCreateInfo.hpp"
#include "cm/TerrainResult.hpp"

namespace tr {

class SdfGenerator {
public:
  SdfGenerator();
  ~SdfGenerator();

  SdfGenerator(const SdfGenerator&) = delete;
  SdfGenerator(SdfGenerator&&) = delete;
  auto operator=(const SdfGenerator&) -> SdfGenerator& = delete;
  auto operator=(SdfGenerator&&) -> SdfGenerator& = delete;

  auto registerSdf(SdfCreateInfo createInfo) -> SdfHandle;

  /// Returns an 8 bit signed integer between -127 and 127 representing the perpendicular below or
  /// above the surface.
  auto sampleSdf(SdfHandle handle, glm::vec3 coords) -> int8_t;

private:
  MapKey keyGenerator;
  std::unordered_map<SdfHandle, SdfCreateInfo> sdfMap;
};

}
