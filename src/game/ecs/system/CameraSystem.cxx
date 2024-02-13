#include "CameraSystem.hpp"

#include "game/actions/ActionState.hpp"
#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Resources.hpp"
#include "game/actions/ActionType.hpp"

namespace Triton::Game::Ecs::CameraSystem {

   using namespace Actions;

   const auto CameraSpeed = 0.005f;
   const auto MouseSensitivity = 0.0025f;
   const auto PitchExtent = 89.f;

   void fixedUpdate(entt::registry& registry) {
      const auto view = registry.view<Camera>();
      const auto [width, height] = registry.ctx().get<const WindowDimensions>();

      auto& actionState = registry.ctx().get<ActionState>();

      for (auto [entity, cam] : view.each()) {

         if (actionState.getBool(ActionType::MoveForward)) {
            cam.position += CameraSpeed * cam.front;
         }

         if (actionState.getBool(ActionType::MoveBackward)) {
            cam.position -= CameraSpeed * cam.front;
         }

         if (actionState.getBool(ActionType::StrafeLeft)) {
            cam.position -= glm::normalize(glm::cross(cam.front, cam.cameraUp)) * CameraSpeed;
         }

         if (actionState.getBool(ActionType::StrafeRight)) {
            cam.position += glm::normalize(glm::cross(cam.front, cam.cameraUp)) * CameraSpeed;
         }

         const auto xOffset = actionState.getFloatDelta(ActionType::LookHorizontal);
         const auto yOffset = actionState.getFloatDelta(ActionType::LookVertical);

         cam.yaw += (xOffset * MouseSensitivity);
         cam.pitch += (yOffset * MouseSensitivity);

         cam.pitch = std::min(cam.pitch, PitchExtent);
         cam.pitch = std::max(cam.pitch, -PitchExtent);

         auto direction = glm::vec3{cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch))};

         cam.front = glm::normalize(direction);

         cam.view = glm::lookAt(cam.position, cam.position + cam.front, {0.f, 1.f, 0.f});

         float aspect = static_cast<float>(width) / static_cast<float>(height);
         cam.projection =
             glm::perspective(glm::radians(cam.fov), aspect, cam.nearClip, cam.farClip);
      }
   }
}