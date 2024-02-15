#include "ActionSystem.hpp"

#include "ActionSet.hpp"
#include "KeyMap.hpp"
#include "game/actions/Action.hpp"
#include <GLFW/glfw3.h>

namespace Triton::Actions {

   ActionSystem::ActionSystem(GLFWwindow& newWindow) : window(newWindow) {
   }

   void ActionSystem::mapKey(Key key, ActionType aType, StateType sType) {
      keyActionMap.insert_or_assign(key, Action{aType, sType});
   }

   void ActionSystem::setMouseState(bool captured) {
      if (captured) {
         firstMouse = true;
      }
   }

   void ActionSystem::mouseButtonCallback([[maybe_unused]] int button,
                                          [[maybe_unused]] int action,
                                          [[maybe_unused]] int mods) {
   }

   void ActionSystem::cursorPosCallback(double xpos, double ypos) {
      const auto deltaX = static_cast<float>(prevX - xpos);
      const auto deltaY = static_cast<float>(prevY - ypos);

      prevX = xpos;
      prevY = ypos;

      if (firstMouse) {
         firstMouse = !firstMouse;
         return;
      }

      if (deltaX != 0) {
         actionDelegate(Action{ActionType::LookHorizontal, StateType::Range, deltaX});
      }

      if (deltaY != 0) {
         actionDelegate(Action{ActionType::LookVertical, StateType::Range, deltaY});
      }
   }

   void ActionSystem::keyCallback(int key,
                                  [[maybe_unused]] int scancode,
                                  int action,
                                  [[maybe_unused]] int mods) {
      auto it = keyMap.find(key);
      if (it == keyMap.end()) {
         Log::warn << "unmapped key found: " << key << std::endl;
         return;
      }
      auto actionKey = keyMap.at(key);

      const auto sourceIt = keyActionMap.find(actionKey);
      if (sourceIt == keyActionMap.end()) {
         return;
      }
      const auto& source = sourceIt->second;

      if (action == GLFW_PRESS) {
         actionDelegate(Action{source.actionType, source.stateType, true});
      } else if (action == GLFW_RELEASE) {
         actionDelegate(Action{source.actionType, source.stateType, false});
      }
   }

   ActionSet& ActionSystem::createActionSet(ActionSets name) {
      auto [iter, inserted] = actionSetMap.emplace(name, ActionSet());
      return iter->second;
   }

   void ActionSystem::setActiveSet(ActionSets newActiveSet) {
      activeSet = newActiveSet;
   }
}
