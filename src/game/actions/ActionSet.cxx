#include "ActionSet.hpp"
#include "ActionType.hpp"
#include "game/events/Key.hpp"
#include "game/events/Mouse.hpp"

namespace Triton::Actions {

   // Map a key event to fire an action
   void ActionSet::mapKey(Key key, ActionType actionType) {
      keyMap.insert(std::make_pair(key, actionType));
   }

   bool ActionSet::hasMapping(const Key key) const {
      return keyMap.contains(key);
   }

   ActionType ActionSet::getActionForKey(const Key key) const {
      const auto it = keyMap.find(key);
      assert(it != keyMap.end());
      return it->second;
   }

   ActionType ActionSet::getActionForMouse(const Events::MouseEvent mouse) const {
      const auto it = mouseMap.find(mouse);
      assert(it != mouseMap.end());
      return it->second;
   }

   ActionType ActionSet::getActionForSourceEvent(const SourceEvent source) const {
      if (auto mouseEvent = std::get_if<Events::MouseEvent>(&source.src)) {
         return getActionForMouse(*mouseEvent);
      } else if (auto key = std::get_if<Key>(&source.src)) {
         return getActionForKey(*key);
      }
      return ActionType::MoveBackward;
   }

   void ActionSet::bindSource(Key key, ActionType actionType) {
      keyMap.insert(std::make_pair(key, actionType));
   }

   void ActionSet::bindSource(Events::MouseEvent mouse, ActionType actionType) {
      mouseMap.insert(std::make_pair(mouse, actionType));
   };

   void ActionSet::bindSource(Source src, ActionType actionType) {
      if (auto mouseEvent = std::get_if<Events::MouseEvent>(&src.src)) {
         mouseMap.insert(std::make_pair(*mouseEvent, actionType));
      } else if (auto key = std::get_if<Key>(&src.src)) {
         keyMap.insert(std::make_pair(*key, actionType));
      }
   }

}
