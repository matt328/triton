#include "tr/SdfGenerator.hpp"
#include "cm/GlmToString.hpp"

namespace tr {

auto SdfGenerator::registerSdf(SdfCreateInfo createInfo) -> SdfHandle {
  const auto key = keyGenerator.getKey();
  sdfMap.emplace(key, createInfo);
  return key;
}

auto SdfGenerator::sampleSdf(SdfHandle handle, glm::vec3 coords) -> int8_t {
  assert(sdfMap.contains(handle) && "Invalid SdfHandle used in sampleSdf!");

  const auto info = sdfMap.at(handle);

  switch (info.shapeType) {
    case ShapeType::Plane:
      return samplePlane(handle, coords);
      break;
    case ShapeType::Box:
      return sampleBox(handle, coords);
      break;
  }
}

auto SdfGenerator::sampleBox(SdfHandle handle, glm::vec3 coords) -> int8_t {
  const auto info = sdfMap.at(handle);
  glm::vec3 q = glm::abs(coords - info.center) - info.size;
  auto sdfValue =
      glm::length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);

  sdfValue *= info.voxelSize;

  return static_cast<int8_t>(std::clamp(sdfValue, MinSdfValue, MaxSdfValue));
}

auto SdfGenerator::samplePlane(SdfHandle handle, glm::vec3 coords) -> int8_t {
  const auto info = sdfMap.at(handle);
  const auto voxelSize = info.voxelSize;

  const auto height = voxelSize - info.height;

  float sdfValue = (coords.y - height);
  sdfValue = std::clamp(sdfValue, -voxelSize, voxelSize);

  auto mappedValue = MinSdfValue + (((sdfValue - (-voxelSize)) / (voxelSize - (-voxelSize))) *
                                    (MaxSdfValue - MinSdfValue));
  return static_cast<int8_t>(mappedValue);
}

auto SdfGenerator::getVoxelSize(SdfHandle handle) -> float {
  assert(sdfMap.contains(handle) && "Invalid SdfHandle used in sampleSdf!");
  const auto info = sdfMap.at(handle);
  return info.voxelSize;
}

}
