#pragma once

namespace tr {
class IDensityGenerator {
public:
  IDensityGenerator() = default;
  virtual ~IDensityGenerator() = default;

  IDensityGenerator(const IDensityGenerator&) = default;
  IDensityGenerator(IDensityGenerator&&) = delete;
  auto operator=(const IDensityGenerator&) -> IDensityGenerator& = default;
  auto operator=(IDensityGenerator&&) -> IDensityGenerator& = delete;

  virtual auto getValue(glm::vec3 position) -> float = 0;
};
}
