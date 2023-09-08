#include "ActionManager.hpp"

namespace Input {
   void ActionManager::mapKey(Key key, ActionType actionType) {
      actionMap.insert(std::make_pair(key, actionType));
   }

   void ActionManager::keyPressed(Key key) {
      const auto it = actionMap.find(key);
      if (it == actionMap.end()) {
         return;
      }

      const auto actionType = it->second;

      const auto delegateIt = delegatesMap.find(actionType);

      if (delegateIt == delegatesMap.end()) {
         return;
      }

      delegateIt->second(Action{actionType, false});
   };
   void ActionManager::keyReleased(Key key) {
   }

   void ActionManager::onAction(ActionType aType, std::function<void(Action)> fn) {
      const auto it = delegatesMap.find(aType);
      if (it != delegatesMap.end()) {
         it->second.addDelegate(fn);
      }
   }
}