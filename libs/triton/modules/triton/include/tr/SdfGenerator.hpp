#pragma once

#include "cm/Rando.hpp"
#include "cm/SdfCreateInfo.hpp"
#include "cm/TerrainResult.hpp"
#include "tr/IDensityGenerator.hpp"

namespace tr {

using GeneratorHandle = uint64_t;

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

  auto registerGenerator(SdfCreateInfo createInfo) -> GeneratorHandle;
  auto getGenerator(GeneratorHandle handle) -> std::shared_ptr<IDensityGenerator>;

private:
  MapKey keyGen;
  std::unordered_map<SdfHandle, std::shared_ptr<IDensityGenerator>> generatorMap;
};

}
