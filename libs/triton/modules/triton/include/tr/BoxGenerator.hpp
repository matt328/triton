#pragma once

#include "IDensityGenerator.hpp"

namespace tr {

class BoxGenerator : public IDensityGenerator {
public:
  BoxGenerator(glm::vec3 newCenter, float newRadius) : center{newCenter}, radius{newRadius} {
  }

  auto getValue(glm::vec3 position) -> float override {
    return glm::length(position - center) - radius;
  }

  auto getValue(float x, float y, float z) -> float override {
    return getValue(glm::vec3(x, y, z));
  }

private:
  glm::vec3 center;
  float radius;
};

}
