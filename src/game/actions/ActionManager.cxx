#include "ActionManager.hpp"
#include "ActionSet.hpp"

namespace Triton::Actions {

   ActionSet& ActionManager::createActionSet(ActionSets name) {
      auto [iter, inserted] = actionSetMap.emplace(name, ActionSet{});
      return iter->second;
   }

   void ActionManager::setActiveSet(ActionSets newActiveSet) {
      activeSet = newActiveSet;
   }

   void ActionManager::keyPressed(Key key) {
      auto& actionSet = actionSetMap.at(activeSet);

      const auto actionType = actionSet.getActionForKey(key);
      auto sourceEvent = SourceEvent{Source{key}};

      

   }

   void ActionManager::keyReleased(Key key) {
   }

}