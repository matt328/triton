#pragma once

namespace tr::gp::cmp {

   constexpr auto DefaultYaw = -90.f;
   constexpr glm::vec3 worldUp = {0.f, 1.f, 0.f};

   struct Camera {
      Camera() = default;
      Camera(const int width,
             const int height,
             const float fov,
             const float nearClip,
             const float farClip,
             const glm::vec3& position)
          : position{position},
            projection(
                glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), nearClip, farClip)),
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
      glm::vec3 direction = normalize(position - target);
      glm::vec3 right = normalize(cross(worldUp, direction));

      glm::vec3 velocity{};

      glm::mat4 projection{};
      glm::mat4 view{};

      int width{}, height{};
      float fov{}, nearClip{}, farClip{};
   };
}