#pragma once

#include "GlfwKeyMap.hpp"
#include "Key.hpp"
#include <GLFW/glfw3.h>

namespace Actions {
   enum class Action : uint32_t {
      MoveForward = 0,
      MoveBackward,
      StrafeLeft,
      StrafeRight
   };
   [[maybe_unused]] static std::string toString(const Action actionId) {
      switch (actionId) {
         case Action::MoveForward:
            return "Move Forward";
         case Action::MoveBackward:
            return "Move Backward";
         case Action::StrafeLeft:
            return "Strafe Left";
         case Action::StrafeRight:
            return "Strafe Right";
         default:
            return "Unknown Action ID";
      }
   };
}

enum class KeyState {
   Up,
   Pressed,
   Down,
   Released
};

using ActionDelegate = entt::delegate<void(const Actions::Action&)>;
// TODO: Remove GLFW dependency from game lib
class InputSystem {
 public:
   InputSystem() : actionMap({}) {

      Input::initializeGlfwKeyMap(keyMap);

      actionMap.insert({{Input::Key::W, Actions::Action::MoveForward},
                        {Input::Key::S, Actions::Action::MoveBackward},
                        {Input::Key::A, Actions::Action::StrafeLeft},
                        {Input::Key::D, Actions::Action::StrafeRight}});

      allActionsMap.insert({{Actions::Action::MoveForward, {Input::Key::W, Input::Key::Up}}});

      actionDelegate = ActionDelegate{};
   };
   InputSystem(const InputSystem&) = default;
   InputSystem(InputSystem&&) = delete;
   InputSystem& operator=(const InputSystem&) = default;
   InputSystem& operator=(InputSystem&&) = delete;
   ~InputSystem() = default;

   void keyCallback(const int key,
                    [[maybe_unused]] int scancode,
                    const int action,
                    [[maybe_unused]] int mods) {
      const auto mappedKey = keyMap.getKey(key);
      if (action == GLFW_PRESS) {
         keyStateMap[mappedKey] = KeyState::Down;
      } else if (action == GLFW_RELEASE) {
         keyStateMap[mappedKey] = KeyState::Up;
      }
   }

   float getActionValue(const Actions::Action& action) {
      return 0.f;
   }

   bool isActionActive(const Actions::Action& action) {
      if (allActionsMap.find(action) != allActionsMap.end()) {
         const auto inputKeys = allActionsMap.at(action);
         for (const auto key : inputKeys) {
            if (keyStateMap.find(key) != keyStateMap.end()) {
               return keyStateMap.at(key) == KeyState::Down;
            }
         }
      }
      return false;
   }

   ActionDelegate& getActionDelegate() {
      return actionDelegate;
   }

 private:
   std::unordered_map<const Actions::Action, std::vector<Input::Key>> allActionsMap = {};
   std::unordered_map<Input::Key, const Actions::Action> actionMap = {};
   ActionDelegate actionDelegate;

   std::unordered_map<Input::Key, KeyState> keyStateMap;

   Input::Bimap<Input::Key, int> keyMap;
};