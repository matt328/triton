#pragma once

#include "api/action/IActionSystem.hpp"
#include "api/action/Sources.hpp"
#include "api/action/Inputs.hpp"

namespace tr {

class IEventBus;

class ActionSystem : public IActionSystem {
public:
  explicit ActionSystem(std::shared_ptr<IEventBus> newEventBus);
  ~ActionSystem() override;

  ActionSystem(const ActionSystem&) = default;
  ActionSystem(ActionSystem&&) = delete;
  auto operator=(const ActionSystem&) -> ActionSystem& = default;
  auto operator=(ActionSystem&&) -> ActionSystem& = delete;

  void mapSource(Source source, StateType sType, ActionType aType) override;

private:
  std::shared_ptr<IEventBus> eventBus;

  double prevX{}, prevY{};
  bool firstMouse = true;
  std::unordered_map<Key, Action> keyActionMap;
  std::unordered_map<MouseInput, Action> mouseActionMap;
};

}
