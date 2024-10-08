#include "CameraSystem.hpp"

#include "components/Camera.hpp"
#include "components/Resources.hpp"

#include "actions/ActionType.hpp"
#include "actions/Action.hpp"

namespace tr::gp::sys::CameraSystem {

   constexpr auto CameraSpeed = .05f;
   constexpr auto MouseSensitivity = 0.025f;
   constexpr auto PitchExtent = 89.f;

   /*
      TODO: this function shouldn't be like this. The system needs to be able to poll for actions
      rather than being told what actions happen, since in a multithreaded env, handleAction
      could get called at the same time as fixedUpdate. So I guess we'd queue up actions in a
      list and fixedUpdate would process them in order before continuing on with the rest of the
      update logic for that specific entity.

      This will affect the action system in general, the system itself should support queueing up
      a list of actions to be dealt with by a separate action system?
      An action system would need to know whether an entity should react to an action or not

      In general, some of these systems won't be able to be multithreaded and will have to know
      that others have run to completion before they do. Ex the action system has to have a chance
      to set an entity's velocity before the TransformSystem calculates its position.
   */
   void handleAction(entt::registry& registry, const Action& action) {

      for (const auto view = registry.view<cmp::Camera>(); auto [entity, cam] : view.each()) {

         if (action.stateType == StateType::State) {
            const auto value = std::get<bool>(action.value);

            if (action.actionType == ActionType::StrafeLeft) {
               cam.velocity.x = value ? -CameraSpeed : 0.f;
            }

            if (action.actionType == ActionType::StrafeRight) {
               cam.velocity.x = value ? CameraSpeed : 0.f;
            }

            if (action.actionType == ActionType::MoveForward) {
               cam.velocity.z = value ? CameraSpeed : 0.f;
            }

            if (action.actionType == ActionType::MoveBackward) {
               cam.velocity.z = value ? -CameraSpeed : 0.f;
            }
         } else if (action.stateType == StateType::Range) {
            const auto value = std::get<float>(action.value);

            if (action.actionType == ActionType::LookHorizontal) {
               cam.yaw -= value * MouseSensitivity;
            }

            if (action.actionType == ActionType::LookVertical) {
               cam.pitch += value * MouseSensitivity; // Invert Y-Axis 4 life
               cam.pitch = std::min(cam.pitch, PitchExtent);
               cam.pitch = std::max(cam.pitch, -PitchExtent);
            }
         }
      }
   }

   void fixedUpdate(entt::registry& registry) {
      const auto view = registry.view<cmp::Camera>();

      const auto [width, height] = registry.ctx().get<const cmp::WindowDimensions>();

      for (auto [entity, cam] : view.each()) {

         auto direction = glm::vec3{cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.pitch)),
                                    sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch))};

         cam.front = normalize(direction);
         cam.right = normalize(cross(cam.front, glm::vec3(0.0f, 1.0f, 0.0f)));

         glm::mat3 rotationMatrix{cam.right, cmp::worldUp, cam.front};

         const auto rotatedVelocity = rotationMatrix * cam.velocity;

         cam.position += rotatedVelocity;

         cam.view = lookAt(cam.position, cam.position + cam.front, {0.f, 1.f, 0.f});

         const float aspect = static_cast<float>(width) / static_cast<float>(height);
         cam.projection =
             glm::perspective(glm::radians(cam.fov), aspect, cam.nearClip, cam.farClip);
         // Apparently everyone except me knew glm was for OpenGL and you have to adjust these
         // matrices for Vulkan
         cam.projection[1][1] *= -1;
      }
   }
}