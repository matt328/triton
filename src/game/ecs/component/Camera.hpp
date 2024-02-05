#pragma once

namespace Triton::Game::Ecs {
   struct Camera {
      uint32_t width{};
      uint32_t height{};

      float fov{};
      float nearClip{};
      float farClip{};

      glm::mat4 view{};
      glm::mat4 projection{};

      glm::vec3 position{};
   };
}