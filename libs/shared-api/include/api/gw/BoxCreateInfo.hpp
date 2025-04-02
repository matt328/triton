#pragma once

#include "api/gw/GameObjectType.hpp"
#include "bk/Color.hpp"

namespace tr {
struct BoxCreateInfo {
  std::string tag;
  glm::vec3 center;
  float extent;
  Color color{Colors::White};
  std::optional<GameObjectId> target{std::nullopt};
  std::optional<glm::vec3> targetOffset{std::nullopt};
};
}
