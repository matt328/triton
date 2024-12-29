#include "gp/action/ActionSystem.hpp"
#include "tr/IEventBus.hpp"

namespace tr {

ActionSystem::ActionSystem(std::shared_ptr<IEventBus> newEventBus)
    : eventBus{std::move(newEventBus)} {

  Log.debug("Creating ActionSystem");

  eventBus->subscribe<MouseCaptured>([&](const MouseCaptured& event) {
    Log.debug("Mouse Capture, captured: {0}", event.isMouseCaptured);
    if (event.isMouseCaptured) {
      firstMouse = true;
    }
  });

  eventBus->subscribe<KeyEvent>([&](const KeyEvent& event) {
    const auto sourceIt = keyActionMap.find(event.key);
    if (sourceIt == keyActionMap.end()) {
      return;
    }

    const auto& [actionType, stateType, value] = sourceIt->second;

    if (event.buttonState == ButtonState::Pressed) {
      eventBus->emit(Action{.actionType = actionType, .stateType = stateType, .value = true});
    } else if (event.buttonState == ButtonState::Released) {
      eventBus->emit(Action{.actionType = actionType, .stateType = stateType, .value = false});
    }
  });

  eventBus->subscribe<MouseMoved>([&](const MouseMoved& event) {
    const auto deltaX = static_cast<float>(prevX - event.x);
    const auto deltaY = static_cast<float>(prevY - event.y);

    prevX = event.x;
    prevY = event.y;

    if (firstMouse) {
      firstMouse = !firstMouse;
      return;
    }

    if (deltaX != 0) {
      if (const auto xit = mouseActionMap.find(MouseInput::MOVE_X); xit != mouseActionMap.end()) {
        eventBus->emit(Action{.actionType = xit->second.actionType,
                              .stateType = xit->second.stateType,
                              .value = deltaX});
      }
    }

    if (deltaY != 0) {
      if (const auto yit = mouseActionMap.find(MouseInput::MOVE_Y); yit != mouseActionMap.end()) {
        eventBus->emit(Action{.actionType = yit->second.actionType,
                              .stateType = yit->second.stateType,
                              .value = deltaY});
      }
    }
  });
}

ActionSystem::~ActionSystem() {
  keyActionMap.clear();
  mouseActionMap.clear();
}

void ActionSystem::mapSource(Source source, StateType sType, ActionType aType) {
  std::visit(
      [&]<typename T>(T&& arg) {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, Key>) {
          keyActionMap.insert_or_assign(arg, Action{.actionType = aType, .stateType = sType});
        } else if constexpr (std::is_same_v<U, MouseInput>) {
          mouseActionMap.insert_or_assign(arg, Action{.actionType = aType, .stateType = sType});
        } else if constexpr (std::is_same_v<U, GamepadInput>) {
        }
      },
      source.src);
}

}
