#pragma once

namespace Triton::Game::Ecs {
   struct Camera {
      Camera(int width, int height, float fov, float nearClip, float farClip)
          : projection(glm::perspective(fov, (float)(width / height), nearClip, farClip)),
            width(width),
            height(height),
            fov(fov),
            nearClip(nearClip),
            farClip(farClip) {
      }
      float yaw{0.f}, pitch{0.f};
      glm::vec3 direction{0.f, 0.f, -1.f};

      glm::vec3 position{0.f, 0.f, 0.f};
      float velocity{};

      glm::mat4 projection{};
      glm::mat4 view{};

      int width{}, height{};
      float fov{}, nearClip{}, farClip{};
   };
}