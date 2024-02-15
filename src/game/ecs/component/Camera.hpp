#pragma once

namespace Triton::Game::Ecs {

   const auto DefaultYaw = -90.f;
   constexpr glm::vec3 worldUp = {0.f, 1.f, 0.f};

   struct Camera {
      Camera(int width, int height, float fov, float nearClip, float farClip, glm::vec3 position)
          : position{position},
            projection(
                glm::perspective(glm::radians(fov), (float)(width / height), nearClip, farClip)),
            width(width),
            height(height),
            fov(fov),
            nearClip(nearClip),
            farClip(farClip) {
      }
      float yaw{DefaultYaw}, pitch{0.f};

      glm::vec3 position{};
      glm::vec3 front{0.f, 0.f, -1.f};
      glm::vec3 cameraUp = worldUp; // for now

      glm::vec3 target{0.f, 0.f, 0.f};
      glm::vec3 direction = glm::normalize(position - target);
      glm::vec3 right = glm::normalize(glm::cross(worldUp, direction));

      glm::vec3 velocity{};

      glm::mat4 projection{};
      glm::mat4 view{};

      int width{}, height{};
      float fov{}, nearClip{}, farClip{};
   };
}