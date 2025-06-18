#pragma once

#include "api/action/Actions.hpp"
#include "components/Camera.hpp"

namespace tr {

class CameraHandler {
public:
  CameraHandler(const CameraHandler&) = default;
  CameraHandler(CameraHandler&&) = delete;
  auto operator=(const CameraHandler&) -> CameraHandler& = default;
  auto operator=(CameraHandler&&) -> CameraHandler& = delete;

  static auto handleAction(const Action& action, entt::registry& registry) -> void;

private:
  CameraHandler() = default;
  ~CameraHandler() = default;
  static auto handleStateAction(const Action& action, Camera& cam) -> void;
  static auto handleRangeAction(const Action& action, Camera& cam) -> void;
};

}
