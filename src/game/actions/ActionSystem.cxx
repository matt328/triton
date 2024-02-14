#include "ActionSystem.hpp"

#include "ActionSet.hpp"
#include "game/actions/ActionType.hpp"
#include "game/actions/KeyMap.hpp"
#include "game/actions/Mouse.hpp"
#include <GLFW/glfw3.h>

namespace Triton::Actions {

   void ActionSystem::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
      // Need to reverse the keymap (again)
      const auto actionKey = keyMap.at(key);
      auto bindings = std::unordered_map<Source, ActionType>{};
      for (const auto& it : bindings) {
         const auto source = it.first;
         const auto actionType = it.second;
         // TODO: if 2 keys for the same action are down, releasing either of them will
         // set the action to false. Might should do something about this, but would require polling
         // the other keys associated with this action.  Bindings would have to be a bimap.
         if (bindingMatches(actionKey)) {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
               actionState.setBool(actionType, true);
            } else if (action == GLFW_RELEASE) {
               actionState.setBool(actionType, false);
            }
         }
      }
      // Loop through all the bindings and see if this key is involved
   }

   void ActionSystem::update() {
      actionState.nextFrame(frameNumber);
      // Iterate the map, for each action, calculate the ActionState for it's set of sources
      {
         const auto& boolMap = actionSetMap.at(activeSet).getBoolMap();
         for (const auto& it : boolMap) {
            const auto actionType = it.first;
            auto range = boolMap.equal_range(actionType);
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
         // Set the current float value in the ActionStateMap regardless of its value
         const auto& floatMap = actionSetMap.at(activeSet).getFloatMap();
         for (const auto& it : floatMap) {
            const auto actionType = it.first;
            auto range = floatMap.equal_range(actionType);
            for (auto& i = range.first; i != range.second; ++i) {
               const auto value = sourceToFloat(i->second);
               actionState.setFloat(actionType, value);
            }
         }
      }
      frameNumber++;
   }

   float ActionSystem::sourceToFloat(const Source& source) const {
      auto mouseInput = std::get_if<MouseInput>(&source.src);
      if (mouseInput != nullptr) {
         if (*mouseInput == MouseInput::MOVE_X || *mouseInput == MouseInput::MOVE_Y) {
            double x{}, y{};
            glfwGetCursorPos(&window, &x, &y);
            if (*mouseInput == MouseInput::MOVE_X) {
               return static_cast<float>(x);
            } else if (*mouseInput == MouseInput::MOVE_Y) {
               return static_cast<float>(y);
            }
         }
      }
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
