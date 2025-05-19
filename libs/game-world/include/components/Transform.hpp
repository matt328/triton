#pragma once

namespace tr {

struct Transform {
  glm::quat rotation{glm::identity<glm::quat>()};
  glm::vec3 position{glm::zero<glm::vec3>()};
  glm::vec3 scale{glm::one<glm::vec3>()};
};

}
