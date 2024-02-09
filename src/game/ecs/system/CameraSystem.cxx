#include "CameraSystem.hpp"

#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Resources.hpp"
#include "game/Actions.hpp"

namespace Triton::Game::Ecs::CameraSystem {
   void fixedUpdate(entt::registry& registry) {
      const auto view = registry.view<Camera>();
      const auto [width, height] = registry.ctx().get<const WindowDimensions>();

      auto& mainMap = registry.ctx().get<gainput::InputMap>();

      for (auto [entity, cam] : view.each()) {
         // TODO: Update camera position, yaw and pitch based on input state from a context variable
         if (mainMap.GetBool(Actions::MoveForward)) {
            cam.position = cam.position + (cam.direction * 0.005f);
         }
         if (mainMap.GetBool(Actions::MoveBackward)) {
            cam.position = cam.position - (cam.direction * 0.005f);
         }

         auto rightVector = glm::cross(cam.position, {0.f, 1.f, 0.f});
         if (mainMap.GetBool(Actions::StrafeLeft)) {
            cam.position = cam.position + (rightVector * 0.005f);
         }

         if (mainMap.GetBool(Actions::StrafeRight)) {
            cam.position = cam.position - (rightVector * 0.005f);
         }

         // Update the camera's direction vector based on yaw and pitch
         // This needs to be kept up to date for future movement
         glm::quat orientation =
             glm::angleAxis(glm::radians(cam.yaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
             glm::angleAxis(glm::radians(cam.pitch), glm::vec3(1.0f, 0.0f, 0.0f));

         cam.direction = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, -1.0f)));

         // Update the camera's view
         cam.view = glm::lookAt(cam.position, cam.direction, {0.f, 1.f, 0.f});

         // Update the camera's projection matrix
         cam.width = width;
         cam.height = height;
         cam.projection = glm::perspective(cam.fov,
                                           (float)((float)cam.width / (float)cam.height),
                                           cam.nearClip,
                                           cam.farClip);
      }
   }
}