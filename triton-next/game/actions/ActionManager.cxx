#include "ActionManager.hpp"
#include "ActionType.hpp"
#include "ActionSet.hpp"

namespace Triton::Actions {

   std::shared_ptr<ActionSet> ActionManager::getCurrentActionSet() const {
      return currentActionSet;
   }

   bool ActionManager::hasMapping(const Key key) const {
      return getCurrentActionSet()->hasMapping(key);
   }

   ActionType ActionManager::mapKeyToAction(const Key key) const {
      return getCurrentActionSet()->mapKeyToAction(key);
   }

   void ActionManager::setCurrentActionSet(const std::shared_ptr<ActionSet>& newCurrent) {
      currentActionSet = newCurrent;
   }

}