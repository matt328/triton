#pragma once

#include "gp/components/Camera.hpp"
#include "tr/IEventBus.hpp"

namespace tr {

struct Action;

class CameraSystem {
public:
  explicit CameraSystem(const std::shared_ptr<IEventBus>& eventBus, entt::registry& registry);
  ~CameraSystem();

  CameraSystem(const CameraSystem&) = default;
  CameraSystem(CameraSystem&&) = delete;
  auto operator=(const CameraSystem&) -> CameraSystem& = default;
  auto operator=(CameraSystem&&) -> CameraSystem& = delete;

  static void fixedUpdate(entt::registry& registry);

private:
  static auto handleAction(const Action& action, entt::registry& registry) -> void;
  static auto handleStateAction(const Action& action, Camera& cam) -> void;
  static auto handleRangeAction(const Action& action, Camera& cam) -> void;
};

}
