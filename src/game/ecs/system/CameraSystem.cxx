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

   void handleAction(entt::registry& registry, Actions::Action& action) {
      Log::info << "Action Recieved: " << std::endl;

      /*
         This Action should be enhanced to say that it's a 'state' type of action
         if the state is 'on' we should set the velocity here, not the position.
         set the position in the update method.
      */

      const auto view = registry.view<Camera>();
      for (auto [entity, cam] : view.each()) {

         if (action.actionType == ActionType::StrafeLeft) {
            auto value = std::get<bool>(action.value);
            cam.position -= glm::normalize(glm::cross(cam.front, cam.cameraUp)) * CameraSpeed;
         }

         if (action.actionType == ActionType::StrafeLeft) {
            cam.position += glm::normalize(glm::cross(cam.front, cam.cameraUp)) * CameraSpeed;
         }
      }
   }

   void fixedUpdate(entt::registry& registry) {
      const auto view = registry.view<Camera>();
      const auto [width, height] = registry.ctx().get<const WindowDimensions>();

      auto& actionState = registry.ctx().get<ActionState>();

      for (auto [entity, cam] : view.each()) {

         // if (actionState.getBool(ActionType::MoveForward)) {
         //    cam.position += CameraSpeed * cam.front;
         // }

         // if (actionState.getBool(ActionType::MoveBackward)) {
         //    cam.position -= CameraSpeed * cam.front;
         // }

         // if (actionState.getBool(ActionType::StrafeLeft)) {
         //    cam.position -= glm::normalize(glm::cross(cam.front, cam.cameraUp)) * CameraSpeed;
         // }

         // const auto xOffset = actionState.getFloatDelta(ActionType::LookHorizontal);
         // const auto yOffset = actionState.getFloatDelta(ActionType::LookVertical);

         // cam.yaw += (xOffset * MouseSensitivity);
         // cam.pitch -= (yOffset * MouseSensitivity);

         // cam.pitch = std::min(cam.pitch, PitchExtent);
         // cam.pitch = std::max(cam.pitch, -PitchExtent);

         auto direction = glm::vec3{cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch))};

         cam.front = glm::normalize(direction);

         cam.view = glm::lookAt(cam.position, cam.position + cam.front, {0.f, 1.f, 0.f});

         float aspect = static_cast<float>(width) / static_cast<float>(height);
         cam.projection =
             glm::perspective(glm::radians(cam.fov), aspect, cam.nearClip, cam.farClip);
         // Apparently everyone except me knew glm was for OpenGL and you have to adjust these
         // matrices for Vulkan
         cam.projection[1][1] *= -1;
      }
   }
}