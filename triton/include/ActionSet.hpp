#pragma once

#include "Delegates.hpp"
#include "Events.hpp"
#include "Key.hpp"
#include "ActionType.hpp"

// Create ActionLayer class, have the actionSet still do most of what it does
// but have action layers be separate sets of actions that can mapKey then add/Rmoeve
// ActionListeners

namespace Triton::Actions {
   class ActionSet {
    public:
      ActionSet() = default;
      ActionSet(const ActionSet&) = delete;
      ActionSet(ActionSet&&) = delete;
      ActionSet& operator=(const ActionSet&) = default;
      ActionSet& operator=(ActionSet&&) = delete;

      virtual ~ActionSet() = default;

      virtual void mapKey(Key key, ActionType actionType) = 0;
      virtual bool hasMapping(Key key) = 0;

      [[nodiscard]] virtual ActionType mapKeyToAction(Key key) const = 0;
   };
};
