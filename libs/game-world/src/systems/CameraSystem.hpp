#pragma once

#include "EntityService.hpp"
#include "components/Camera.hpp"
#include "api/fx/IEventBus.hpp"

namespace tr {

struct Action;

class CameraHandler {
public:
  CameraHandler(const std::shared_ptr<IEventBus>& eventBus,
                std::shared_ptr<EntityService> newEntityService);
  ~CameraHandler();

  CameraHandler(const CameraHandler&) = delete;
  CameraHandler(CameraHandler&&) = delete;
  auto operator=(const CameraHandler&) -> CameraHandler& = delete;
  auto operator=(CameraHandler&&) -> CameraHandler& = delete;

  void fixedUpdate();

private:
  auto handleAction(const Action& action) -> void;
  static auto handleStateAction(const Action& action, Camera& cam) -> void;
  static auto handleRangeAction(const Action& action, Camera& cam) -> void;

  std::shared_ptr<EntityService> entityService;
};

}
