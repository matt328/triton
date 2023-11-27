#include "DefaultActionSet.hpp"
#include "ActionType.hpp"
#include "events/ActionEvent.hpp"

namespace Triton::Actions {

   // Map a key event to fire an action
   void DefaultActionSet::mapKey(Key key, ActionType actionType) {
      actionMap.insert(std::make_pair(key, actionType));
   }

   bool DefaultActionSet::hasMapping(Key key) {
      return actionMap.contains(key);
   }

   ActionType DefaultActionSet::mapKeyToAction(Key key) const {
      const auto it = actionMap.find(key);
      assert(it != actionMap.end());
      return it->second;
   }

}