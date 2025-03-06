#include "tr/SdfGenerator.hpp"

namespace tr {

SdfGenerator::SdfGenerator() {
}

SdfGenerator::~SdfGenerator() {
}

auto SdfGenerator::registerSdf(SdfCreateInfo createInfo) -> SdfHandle {
  const auto key = keyGenerator.getKey();
  sdfMap.emplace(key, createInfo);
  return key;
}

auto SdfGenerator::sampleSdf(SdfHandle handle, glm::vec3 coords) -> int8_t {
  assert(sdfMap.contains(handle) && "Invalid SdfHandle used in sampleSdf!");
  const auto info = sdfMap.at(handle);

  constexpr float maxSdfValue = 127.0f;
  constexpr float minSdfValue = -127.0f;

  float sdfValue = (coords.y - info.height) / info.voxelSize; // Normalize by voxel size
  return static_cast<int8_t>(std::clamp(sdfValue, minSdfValue, maxSdfValue));
}

}
