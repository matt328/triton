#include "ActionManager.hpp"
#include "ActionType.hpp"
#include "ActionSet.hpp"

namespace Triton::Actions {

   ActionSet& ActionManager::getCurrentActionSet() const {
      return *actionSets[currentActionSet];
   }

   bool ActionManager::hasMapping(const Key key) const {
      return getCurrentActionSet().hasMapping(key);
   }

   ActionType ActionManager::mapKeyToAction(const Key key) const {
      return getCurrentActionSet().mapKeyToAction(key);
   }

   size_t ActionManager::createActionSet() {
      actionSets.emplace_back(std::make_unique<ActionSet>());
      currentActionSet = actionSets.size() - 1;
      return currentActionSet;
   }

   void ActionManager::removeActionSet(const size_t id) {
      assert(id != currentActionSet);

      if (id < actionSets.size()) {
         actionSets.erase(actionSets.begin() + id);
      }
   }

   void ActionManager::setCurrentActionSet(const size_t id) {
      currentActionSet = id;
   }

}