#pragma once

#include "IDensityGenerator.hpp"

namespace tr {

class SphereGenerator : public IDensityGenerator {

public:
  SphereGenerator(glm::vec3 newCenter, float newRadius) : center{newCenter}, radius{newRadius} {
  }

  auto getValue(glm::vec3 position) -> float override {
    glm::vec3 q = glm::abs(position - center) - radius;
    return glm::length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
  }

  auto getValue(float x, float y, float z) -> float override {
    return getValue(glm::vec3(x, y, z));
  }

private:
  glm::vec3 center;
  float radius;
};

}
