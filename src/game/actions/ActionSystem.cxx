#include "ActionSystem.hpp"

#include "ActionSet.hpp"
#include "game/actions/KeyMap.hpp"
#include <limits>

namespace Triton::Actions {

   void ActionSystem::update() {
      actionState.nextFrame();
      // Iterate the map, for each action, calculate the ActionState for it's set of sources
      {
         const auto& actionTypeMap = actionSetMap.at(activeSet).getBoolMap();
         for (const auto& it : actionTypeMap) {
            const auto actionType = it.first;
            auto range = actionTypeMap.equal_range(actionType);
            auto set = false;
            for (auto& i = range.first; i != range.second; ++i) {
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
      {
         const auto& floatMap = actionSetMap.at(activeSet).getFloatMap();
         for (const auto& it : floatMap) {
            const auto actionType = it.first;
            auto range = floatMap.equal_range(actionType);
            auto set = false;
            for (auto& i = range.first; i != range.second; ++i) {
               const auto value = sourceToFloat(i->second);
               if (std::abs(value - 0.f) <
                   std::numeric_limits<float>::epsilon()) { // look out big brain time here
                  actionState.setFloat(actionType, value);
                  set = true;
                  continue; // only a single true is needed to set the ActionState to true
               }
            }
            if (!set) {
               // None were set this time, set ActionState to false this frame
               actionState.setFloat(actionType, 0.f);
            }
         }
      }
   }

   float ActionSystem::sourceToFloat(const Source& source) const {
      return 0.f;
   }

   bool ActionSystem::sourceToBool(const Source& source) const {
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
