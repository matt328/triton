#pragma once

#include "EntityService.hpp"
#include "components/Camera.hpp"
#include "fx/IEventBus.hpp"

namespace tr {

struct Action;

class CameraSystem {
public:
  explicit CameraSystem(const std::shared_ptr<IEventBus>& eventBus,
                        std::shared_ptr<EntityService> newEntityService);
  ~CameraSystem();

  CameraSystem(const CameraSystem&) = delete;
  CameraSystem(CameraSystem&&) = delete;
  auto operator=(const CameraSystem&) -> CameraSystem& = delete;
  auto operator=(CameraSystem&&) -> CameraSystem& = delete;

  void fixedUpdate();

private:
  auto handleAction(const Action& action) -> void;
  static auto handleStateAction(const Action& action, Camera& cam) -> void;
  static auto handleRangeAction(const Action& action, Camera& cam) -> void;

  std::shared_ptr<EntityService> entityService;
};

}
