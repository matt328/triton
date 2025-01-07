#pragma once

#include "gp/action/IActionSystem.hpp"

#include "tr/Inputs.hpp"

namespace tr {
class IEventBus;
}

namespace tr {

class ActionSystem final : public IActionSystem {
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
