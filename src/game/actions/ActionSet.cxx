#include "ActionSet.hpp"

namespace Triton::Actions {

   void ActionSet::bindSource(Source source, ActionType actionType) {
      actionTypeMap.insert({actionType, source});
   }
}