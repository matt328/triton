#pragma once

#include "gp/Registry.hpp"
#include "gp/components/Camera.hpp"
#include "tr/IEventBus.hpp"

namespace tr {
struct Action;
}

namespace tr {
class CameraSystem {
public:
  explicit CameraSystem(std::shared_ptr<IEventBus> newEventBus,
                        std::shared_ptr<Registry> newRegistry);
  ~CameraSystem();

  CameraSystem(const CameraSystem&) = default;
  CameraSystem(CameraSystem&&) = delete;
  auto operator=(const CameraSystem&) -> CameraSystem& = default;
  auto operator=(CameraSystem&&) -> CameraSystem& = delete;

  void fixedUpdate() const;

private:
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<Registry> registry;

  auto handleAction(const Action& action) const -> void;
  static auto handleStateAction(const Action& action, Camera& cam) -> void;
  static auto handleRangeAction(const Action& action, Camera& cam) -> void;
};
}
