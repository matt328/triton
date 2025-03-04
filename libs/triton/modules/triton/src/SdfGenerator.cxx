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
  float sdfValue = coords.y - info.height;
  return static_cast<int8_t>(std::clamp(sdfValue, -127.f, 127.f));
}

}
