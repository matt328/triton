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

  static auto handleAction(const std::shared_ptr<Action>& action, entt::registry& registry) -> void;

private:
  CameraHandler() = default;
  ~CameraHandler() = default;
  static auto handleStateAction(const std::shared_ptr<Action>& action, Camera& cam) -> void;
  static auto handleRangeAction(const std::shared_ptr<Action>& action, Camera& cam) -> void;
};

}
