#pragma once

#include "cm/Rando.hpp"
#include "cm/SdfCreateInfo.hpp"
#include "cm/TerrainResult.hpp"

namespace tr {

constexpr float MaxSdfValue = 127.0f;
constexpr float MinSdfValue = -127.0f;

class SdfGenerator {
public:
  SdfGenerator() = default;
  ~SdfGenerator() = default;

  SdfGenerator(const SdfGenerator&) = delete;
  SdfGenerator(SdfGenerator&&) = delete;
  auto operator=(const SdfGenerator&) -> SdfGenerator& = delete;
  auto operator=(SdfGenerator&&) -> SdfGenerator& = delete;

  auto registerSdf(SdfCreateInfo createInfo) -> SdfHandle;

  /// Returns an 8 bit signed integer between -127 and 127 representing the perpendicular below or
  /// above the surface.
  auto sampleSdf(SdfHandle handle, glm::vec3 coords) -> int8_t;

  auto getVoxelSize(SdfHandle handle) -> float;

private:
  MapKey keyGenerator;
  std::unordered_map<SdfHandle, SdfCreateInfo> sdfMap;

  auto sampleBox(SdfHandle handle, glm::vec3 coords) -> int8_t;
  auto samplePlane(SdfHandle handle, glm::vec3 coords) -> int8_t;
};

}
