#include "ActionSystem.hpp"

#include "ActionSet.hpp"
#include "game/actions/KeyMap.hpp"

namespace Triton::Actions {

   void ActionSystem::update() {
      // There are a lot of allocations here that need dealt with.
      // Might need to rethink the approach of this being infinitely flexible
      // basically just need to query glfw state in the ecs, maybe just toss the window in a
      // context variable
      // move current state into previous state, clear current state
      actionState.nextFrame();
      // Iterate the actionMap, for each action, calculate the ActionState for it's set of sources
      auto actionTypeMap = actionSetMap.at(activeSet).getActionTypeMap();

      for (auto& it : actionTypeMap) {
         auto actionType = it.first;
         auto range = actionTypeMap.equal_range(actionType);
         auto set = false;
         for (auto i = range.first; i != range.second; ++i) {
            if (sourceToBool(i->second)) {
               actionState.setBool(actionType, true);
               set = true;
               continue; // only a single true is needed to set the ActionState to true
            }
         }
         if (!set) {
            // None were set this time, set ActionState to false this frame
            actionState.setBool(actionType, false);
         }
      }
   }

   bool ActionSystem::sourceToBool(const Source source) const {
      // Key
      auto key = std::get_if<Key>(&source.src);
      if (key != nullptr) {
         const auto glfwKey = keyMap.at(*key);
         if (glfwGetKey(&window, glfwKey) == GLFW_PRESS) {
            return true;
         }
      }
      return false;
   }

   ActionSet& ActionSystem::createActionSet(ActionSets name) {
      auto [iter, inserted] = actionSetMap.emplace(name, ActionSet());
      return iter->second;
   }

   void ActionSystem::setActiveSet(ActionSets newActiveSet) {
      activeSet = newActiveSet;
   }
}
