#include "ActionSystem.hpp"

#include "ActionSet.hpp"
#include "KeyMap.hpp"

namespace Triton::Actions {

   void ActionSystem::update() {
      // rotate currentState into previousState
      // Move over ActionState, create currentState as a map of ActionType to ActionState
      // Check Keys
      for (const auto& pair : actionSetMap.at(activeSet).getKeyMap()) {
         const auto key = pair.first;
         const auto glfwKey = keyMap.at(key);
         int state = glfwGetKey(&window, glfwKey);
         if (state == GLFW_PRESS) {}
      }

      // Check Mouses

      // Check Gamepads
   }

   ActionSet& ActionSystem::createActionSet(ActionSets name) {
      auto [iter, inserted] = actionSetMap.emplace(name, ActionSet{});
      return iter->second;
   }

   void ActionSystem::setActiveSet(ActionSets newActiveSet) {
      activeSet = newActiveSet;
   }
}
