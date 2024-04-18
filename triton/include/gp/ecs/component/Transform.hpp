#pragma once

namespace tr::gp::ecs {
   struct Transform {
      glm::vec3 rotation{};
      glm::vec3 position{};
      glm::mat4 transformation{};
   };
}
