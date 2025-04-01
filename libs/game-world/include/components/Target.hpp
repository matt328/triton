#pragma once

namespace tr {
struct Target {
  entt::entity targetId;
  glm::vec3 offset{glm::vec3{0.f, 0.f, 0.f}};
};
}
