#include "DefaultActionManager.hpp"

namespace Triton::Actions {

   // Map a key event to fire an action
   void DefaultActionManager::mapKey(Key key, ActionType actionType) {
      actionMap.insert(std::make_pair(key, actionType));
   }

   // Call this when a Key is pressed
   void DefaultActionManager::keyPressed(Key key) {
      const auto it = actionMap.find(key);
      if (it == actionMap.end()) {
         return;
      }

      const auto actionType = it->second;

      const auto delegateIt = delegatesMap.find(actionType);

      if (delegateIt == delegatesMap.end()) {
         return;
      }

      // TODO: how to figure out what payload goes with what actions here?
      // Maybe there are different delegateMaps for different types?
      // KeyPressed -> bool for is key repeated
      // JoystickAnalogAction -> vector for analog input state
      // MouseMoved -> vector for mouse delta from last frame
      delegateIt->second(Action{actionType, false});
   };

   // Call this when a key is released
   void DefaultActionManager::keyReleased(Key key) {
   }

   size_t DefaultActionManager::addActionListener(ActionType aType,
                                                  std::function<void(Action)> fn) {
      const auto it = delegatesMap.find(aType);
      size_t pos = -1;
      if (it == delegatesMap.end()) {
         auto del = Delegate{};
         pos = del.addDelegate(fn);
         delegatesMap.emplace(aType, std::move(del));
      } else {
         pos = it->second.addDelegate(fn);
      }
      return pos;
   }

   void DefaultActionManager::removeActionListener(ActionType aType, size_t position) {
      const auto it = delegatesMap.find(aType);
      assert(it != delegatesMap.end());
      it->second.removeDelegate(position);
   }
}