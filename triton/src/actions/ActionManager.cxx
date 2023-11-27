#include "ActionManager.hpp"
#include "ActionType.hpp"
#include "DefaultActionSet.hpp"

namespace Triton::Actions {

   DefaultActionSet& ActionManager::getCurrentActionSet() const {
      return *actionSets[currentActionSet];
   }

   bool ActionManager::hasMapping(Key key) const {
      return getCurrentActionSet().hasMapping(key);
   }

   ActionType ActionManager::mapKeyToAction(Key key) const {
      return getCurrentActionSet().mapKeyToAction(key);
   }

   size_t ActionManager::createActionSet() {
      actionSets.emplace_back(std::make_unique<DefaultActionSet>());
      currentActionSet = actionSets.size() - 1;
      return currentActionSet;
   }

   void ActionManager::removeActionSet(size_t id) {
      assert(id != currentActionSet);

      if (id < actionSets.size()) {
         actionSets.erase(actionSets.begin() + id);
      }
   }

   void ActionManager::setCurrentActionSet(size_t id) {
      currentActionSet = id;
   }

}