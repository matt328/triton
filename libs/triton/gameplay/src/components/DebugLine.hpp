#pragma once

namespace tr::gp::cmp {

   struct DebugLine {
      DebugLine() = default;
      DebugLine(const glm::vec3 start, const glm::vec3 end) : start(start), end(end) {
      }

      glm::vec3 start;
      glm::vec3 end;
   };

}