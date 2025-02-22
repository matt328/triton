#pragma once

#include "gp/components/Camera.hpp"
#include "tr/IEventBus.hpp"

namespace tr {

struct Action;

class CameraSystem {
public:
  explicit CameraSystem(const std::shared_ptr<IEventBus>& eventBus, entt::registry& registry);
  ~CameraSystem();

  CameraSystem(const CameraSystem&) = delete;
  CameraSystem(CameraSystem&&) = delete;
  auto operator=(const CameraSystem&) -> CameraSystem& = delete;
  auto operator=(CameraSystem&&) -> CameraSystem& = delete;

  void fixedUpdate(entt::registry& registry);

private:
  auto handleAction(const Action& action, entt::registry& registry) -> void;
  static auto handleStateAction(const Action& action, Camera& cam) -> void;
  static auto handleRangeAction(const Action& action, Camera& cam) -> void;

  mutable TracyLockableN(std::shared_mutex, registryMutex, "CameraSystem");
};

}
