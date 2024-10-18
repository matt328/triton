#pragma once

namespace tr::gp::cmp {
   struct DebugAABB {
      DebugAABB() = default;
      DebugAABB(const glm::vec3 min, const glm::vec3 max) : min(min), max(max) {
      }

      glm::vec3 min;
      glm::vec3 max;
   };

}
