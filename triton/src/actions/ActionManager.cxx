#include "ActionManager.hpp"
#include "DefaultActionSet.hpp"

namespace Triton::Actions {

   DefaultActionSet& ActionManager::getCurrentActionSet() {
      return *actionSets[currentActionSet];
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

   void ActionManager::keyPressed(Key key) {
      getCurrentActionSet().keyPressed(key);
   }

   void ActionManager::keyReleased(Key key) {
      getCurrentActionSet().keyReleased(key);
   }

}