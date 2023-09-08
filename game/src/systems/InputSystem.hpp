#pragma once

#include "GlfwKeyMap.hpp"
#include "Key.hpp"
#include "KeyState.hpp"
#include "ActionManager.hpp"

// TODO: Remove GLFW dependency from game lib
class InputSystem {
 public:
   InputSystem() {

      Input::ActionManager actionMapper{};
      actionMapper.mapKey(Input::Key::Up, Input::ActionType::MoveForward);
      actionMapper.mapKey(Input::Key::W, Input::ActionType::MoveForward);

      actionMapper.onAction(Input::ActionType::MoveForward,
                            [](Input::Action a) -> void { const auto b = a.getBooleanValue(); });
   };

   InputSystem(const InputSystem&) = default;
   InputSystem(InputSystem&&) = delete;
   InputSystem& operator=(const InputSystem&) = default;
   InputSystem& operator=(InputSystem&&) = delete;
   ~InputSystem() = default;
};