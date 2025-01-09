#pragma once

namespace tr {

struct GpuCameraData {
  glm::mat4 view{};
  glm::mat4 proj{};
  glm::mat4 viewProj{};
  glm::vec4 position{};
};

}
