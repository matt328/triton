#include "ActionSystem.hpp"
#include "api/fx/IEventQueue.hpp"

namespace tr {

ActionSystem::ActionSystem(std::shared_ptr<IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {

  Log.debug("Creating ActionSystem");

  eventQueue->subscribe<MouseCaptured>([&](const MouseCaptured& event) {
    if (event.isMouseCaptured) {
      firstMouse = true;
    }
  });

  eventQueue->subscribe<KeyEvent>([&](const KeyEvent& event) {
    const auto sourceIt = keyActionMap.find(event.key);
    if (sourceIt == keyActionMap.end()) {
      return;
    }

    const auto& [actionType, stateType, value] = sourceIt->second;

    if (event.buttonState == ButtonState::Pressed) {
      eventQueue->emit(Action{.actionType = actionType, .stateType = stateType, .value = true});
    } else if (event.buttonState == ButtonState::Released) {
      eventQueue->emit(Action{.actionType = actionType, .stateType = stateType, .value = false});
    }
  });

  eventQueue->subscribe<MouseMoved>([&](const MouseMoved& event) {
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
        eventQueue->emit(Action{.actionType = xit->second.actionType,
                                .stateType = xit->second.stateType,
                                .value = deltaX});
      }
    }

    if (deltaY != 0) {
      if (const auto yit = mouseActionMap.find(MouseInput::MOVE_Y); yit != mouseActionMap.end()) {
        eventQueue->emit(Action{.actionType = yit->second.actionType,
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
  auto visitor = [&]<typename T>(T&& arg) {
    using U = std::decay_t<T>;
    if constexpr (std::is_same_v<U, Key>) {
      keyActionMap.insert_or_assign(arg, Action{.actionType = aType, .stateType = sType});
    } else if constexpr (std::is_same_v<U, MouseInput>) {
      mouseActionMap.insert_or_assign(arg, Action{.actionType = aType, .stateType = sType});
    } else if constexpr (std::is_same_v<U, GamepadInput>) {
    }
  };
  std::visit(visitor, source.src);
}

}
