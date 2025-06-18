#include "CameraHandler.hpp"

namespace tr {

constexpr auto CameraSpeed = .010f;
constexpr auto MouseSensitivity = 0.025f;
constexpr auto PitchExtent = 89.f;

auto CameraHandler::handleAction(const Action& action, entt::registry& registry) -> void {
  for (const auto view = registry.view<Camera>(); auto [entity, cam] : view.each()) {
    if (action.stateType == StateType::State) {
      handleStateAction(action, cam);
    } else if (action.stateType == StateType::Range) {
      handleRangeAction(action, cam);
    }
  }
}

auto CameraHandler::handleStateAction(const Action& action, Camera& cam) -> void {
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

auto CameraHandler::handleRangeAction(const Action& action, Camera& cam) -> void {
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

}
