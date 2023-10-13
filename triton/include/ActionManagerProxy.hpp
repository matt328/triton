#pragma once

#include "Delegates.hpp"
#include "Key.hpp"
#include "ActionType.hpp"

namespace Triton::Actions {
   class ActionManagerProxy {
    public:
      virtual ~ActionManagerProxy();

      virtual void mapKey(Key key, ActionType actionType) = 0;

      virtual size_t addActionListener(ActionType aType, std::function<void(Action)> fn) = 0;
      virtual void removeActionListener(ActionType aType, size_t position) = 0;
   };
};
