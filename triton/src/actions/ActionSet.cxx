#include "ActionSet.hpp"
#include "ActionType.hpp"
#include "events/ActionEvent.hpp"

namespace Triton::Actions {

   // Map a key event to fire an action
   void ActionSet::mapKey(Key key, ActionType actionType) {
      actionMap.insert(std::make_pair(key, actionType));
   }

   bool ActionSet::hasMapping(const Key key) const {
      return actionMap.contains(key);
   }

   ActionType ActionSet::mapKeyToAction(const Key key) const {
      const auto it = actionMap.find(key);
      assert(it != actionMap.end());
      return it->second;
   }

}