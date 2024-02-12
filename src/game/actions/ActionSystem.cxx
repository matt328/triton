#include "ActionSystem.hpp"

#include "ActionSet.hpp"
#include "KeyMap.hpp"

namespace Triton::Actions {

   void ActionSystem::update() {
      // move current state into previous state, clear current state
      actionState.nextFrame();
      // Check Keys
      const auto keymap = actionSetMap.at(activeSet).getKeyMap();
      for (const auto& pair : keymap) {
         const auto key = pair.first;
         const auto actionType = pair.second;
         const auto glfwKey = keyMap.at(key);
         int state = glfwGetKey(&window, glfwKey);
         if (state == GLFW_PRESS) {
            actionState.setBool(actionType, true);
         }
         // If a key isn't down, it always returns false
         // so if it detects either of the keys is 'up'
         // the last time through the loop it'll set the action type to false.
         if (state == GLFW_RELEASE) {
            actionState.setBool(actionType, false);
         }
      }

      // Check Mouses

      // Check Gamepads
   }

   ActionSet& ActionSystem::createActionSet(ActionSets name) {
      auto [iter, inserted] = actionSetMap.emplace(name, ActionSet());
      return iter->second;
   }

   void ActionSystem::setActiveSet(ActionSets newActiveSet) {
      activeSet = newActiveSet;
   }
}
