#include "CameraSystem.hpp"

#include "gp/components/Camera.hpp"
#include "components/Resources.hpp"

namespace tr::gp::sys {

   constexpr auto CameraSpeed = .010f;
   constexpr auto MouseSensitivity = 0.025f;
   constexpr auto PitchExtent = 89.f;

   CameraSystem::CameraSystem(std::shared_ptr<IEventBus> newEventBus,
                              std::shared_ptr<Registry> newRegistry)
       : eventBus{std::move(newEventBus)}, registry{std::move(newRegistry)} {

      eventBus->subscribe<Action>([&](const Action& action) { handleAction(action); });
   }

   CameraSystem::~CameraSystem() {
      Log.trace("Destroying CameraSystem");
   }

   auto CameraSystem::handleAction(const Action& action) -> void {
      for (const auto view = registry->getRegistry().view<cmp::Camera>();
           auto [entity, cam] : view.each()) {
         if (action.stateType == StateType::State) {
            CameraSystem::handleStateAction(action, cam);
         } else if (action.stateType == StateType::Range) {
            CameraSystem::handleRangeAction(action, cam);
         }
      }
   }

   auto CameraSystem::handleStateAction(const Action& action, tr::gp::cmp::Camera& cam) -> void {
      const auto value = std::get<bool>(action.value);
      switch (action.actionType) {
         case ActionType::StrafeLeft:
            cam.velocity.x = value ? -CameraSpeed : 0.f;
            break;
         case ActionType::StrafeRight:
            cam.velocity.x = value ? CameraSpeed : 0.f;
            break;
         case ActionType::MoveForward:
            cam.velocity.z = value ? CameraSpeed : 0.f;
            break;
         case ActionType::MoveBackward:
            cam.velocity.z = value ? -CameraSpeed : 0.f;
            break;
         default:
            break;
      }
   }
   auto CameraSystem::handleRangeAction(const Action& action, tr::gp::cmp::Camera& cam) -> void {
      const auto value = std::get<float>(action.value);
      switch (action.actionType) {
         case ActionType::LookHorizontal:
            cam.yaw -= value * MouseSensitivity;
            break;
         case ActionType::LookVertical:
            cam.pitch += value * MouseSensitivity; // Invert Y-Axis 4 life
            cam.pitch = std::min(cam.pitch, PitchExtent);
            cam.pitch = std::max(cam.pitch, -PitchExtent);
            break;
         default:
            break;
      }
   }

   void CameraSystem::fixedUpdate() {
      auto& reg = registry->getRegistry();
      const auto view = reg.view<cmp::Camera>();

      const auto [width, height] = reg.ctx().get<const cmp::WindowDimensions>();

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