#include "ActionSystem.hpp"

#include "ActionSet.hpp"

namespace Triton::Actions {

   ActionSystem::ActionSystem(GLFWwindow& newWindow) : window(newWindow) {
   }

   void ActionSystem::setMouseState(bool captured) {
      if (captured) {
         firstMouse = true;
      }
   }

   void ActionSystem::mouseButtonCallback(int button, int action, int mods) {
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
      if (key == GLFW_KEY_A) {
         if (action == GLFW_PRESS) {
            actionDelegate(Action{ActionType::StrafeLeft, StateType::State, true});
         } else if (action == GLFW_RELEASE) {
            actionDelegate(Action{ActionType::StrafeLeft, StateType::State, false});
         }
      }
      if (key == GLFW_KEY_D) {
         if (action == GLFW_PRESS) {
            actionDelegate(Action{ActionType::StrafeRight, StateType::State, true});
         } else if (action == GLFW_RELEASE) {
            actionDelegate(Action{ActionType::StrafeRight, StateType::State, false});
         }
      }
      if (key == GLFW_KEY_W) {
         if (action == GLFW_PRESS) {
            actionDelegate(Action{ActionType::MoveForward, StateType::State, true});
         } else if (action == GLFW_RELEASE) {
            actionDelegate(Action{ActionType::MoveForward, StateType::State, false});
         }
      }
      if (key == GLFW_KEY_S) {
         if (action == GLFW_PRESS) {
            actionDelegate(Action{ActionType::MoveBackward, StateType::State, true});
         } else if (action == GLFW_RELEASE) {
            actionDelegate(Action{ActionType::MoveBackward, StateType::State, false});
         }
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
