#include "CameraSystem.hpp"

#include "game/actions/ActionState.hpp"
#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Resources.hpp"
#include "game/actions/ActionType.hpp"

namespace Triton::Game::Ecs::CameraSystem {

   using namespace Actions;

   void fixedUpdate(entt::registry& registry) {
      const auto view = registry.view<Camera>();
      const auto [width, height] = registry.ctx().get<const WindowDimensions>();

      auto& actionState = registry.ctx().get<ActionState>();

      const auto cameraSpeed = 0.005f;

      for (auto [entity, cam] : view.each()) {

         if (actionState.getBool(ActionType::MoveForward)) {
            cam.position += cameraSpeed * cam.front;
         }

         if (actionState.getBool(ActionType::MoveBackward)) {
            cam.position -= cameraSpeed * cam.front;
         }

         if (actionState.getBool(ActionType::StrafeLeft)) {
            cam.position -= glm::normalize(glm::cross(cam.front, cam.cameraUp)) * cameraSpeed;
         }

         if (actionState.getBool(ActionType::StrafeRight)) {
            cam.position += glm::normalize(glm::cross(cam.front, cam.cameraUp)) * cameraSpeed;
         }

         const auto xOffset = actionState.getFloatDelta(ActionType::LookHorizontal);
         const auto yOffset = actionState.getFloatDelta(ActionType::LookVertical);

         cam.yaw += xOffset;
         cam.pitch += yOffset;

         cam.yaw = std::min(cam.yaw, 89.f);
         cam.pitch = std::max(cam.pitch, -89.f);

         auto direction = glm::vec3{cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch))};

         cam.front = glm::normalize(direction);

         cam.view = glm::lookAt(cam.position, cam.position + cam.front, cam.cameraUp);

         float aspect = static_cast<float>(width) / static_cast<float>(height);
         cam.projection = glm::perspective(glm::radians(cam.fov), aspect, 0.1f, 1000.f);
      }
   }
}