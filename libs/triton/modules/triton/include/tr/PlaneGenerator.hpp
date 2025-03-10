#pragma once

#include "IDensityGenerator.hpp"

namespace tr {
class PlaneGenerator : public IDensityGenerator {
public:
  PlaneGenerator(glm::vec3 newNormal, float newHeight) : normal{newNormal}, height{newHeight} {
  }

  auto getValue(glm::vec3 position) -> float override {
    return glm::dot(position, normal) + height;
  }

private:
  glm::vec3 normal = glm::vec3(0.f, 1.f, 0.f);
  float height;
};

}
