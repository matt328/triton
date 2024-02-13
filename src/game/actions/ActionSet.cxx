#include "ActionSet.hpp"

namespace Triton::Actions {

   void ActionSet::mapBool(Source source, ActionType actionType) {
      actionTypeMap.insert({actionType, source});
   }

   void ActionSet::mapFloat(Source source, ActionType actionType) {
   }
}