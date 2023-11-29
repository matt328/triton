#pragma once

#include "Key.hpp"
#include "ActionType.hpp"

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
