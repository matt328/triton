#include "ActionState.hpp"

namespace Triton::Actions {

   void ActionState::nextFrame(long long frameNumber) {
      this->frameNumber = frameNumber;
      previousBoolMap = currentBoolMap;
      previousFloatMap = currentFloatMap;
   }

   void ActionState::setBool(ActionType actionType, bool value) {
      currentBoolMap.insert_or_assign(actionType, value);
   }

   void ActionState::setFloat(ActionType actionType, float value) {
      // Prevent a large jump when first capturing mouse
      if (firstMouseX && actionType == ActionType::LookHorizontal) {
         previousFloatMap.insert_or_assign(actionType, value);
         firstMouseX = false;
      }
      if (firstMouseY && actionType == ActionType::LookVertical) {
         previousFloatMap.insert_or_assign(actionType, value);
         firstMouseY = false;
      }
      currentFloatMap.insert_or_assign(actionType, value);
   }

   bool ActionState::getBool(ActionType actionType) const {
      const auto it = currentBoolMap.find(actionType);
      const auto value = (it != currentBoolMap.end() ? it->second : false);
      return value;
   }

   bool ActionState::getBoolWasDown(ActionType actionType) const {
      // working with stl containers is so verbose
      const auto previousIt = previousBoolMap.find(actionType);
      const auto previous = previousIt != previousBoolMap.end() ? previousIt->second : false;

      const auto currentIt = currentBoolMap.find(actionType);
      const auto current = currentIt != currentBoolMap.end() ? currentIt->second : false;

      return previous && !current;
   }

   float ActionState::getFloat(ActionType actionType) const {
      const auto it = currentFloatMap.find(actionType);
      return it != currentFloatMap.end() ? it->second : 0.f;
   }

   float ActionState::getFloatDelta(ActionType actionType) const {
      const auto previousIt = previousFloatMap.find(actionType);
      const auto previous = previousIt != previousFloatMap.end() ? previousIt->second : false;

      const auto currentIt = currentFloatMap.find(actionType);
      const auto current = currentIt != currentFloatMap.end() ? currentIt->second : false;

      return current - previous;
   }

}