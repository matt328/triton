#pragma once

#include "Delegates.hpp"
#include "Key.hpp"
#include "ActionType.hpp"

// Create ActionLayer class, have the ActionManager still do most of what it does
// but have action layers be separate sets of actions that can mapKey then add/Rmoeve
// ActionListeners

namespace Triton::Actions {
   class ActionManager {
    public:
      ActionManager() = default;
      ActionManager(const ActionManager&) = delete;
      ActionManager(ActionManager&&) = delete;
      ActionManager& operator=(const ActionManager&) = default;
      ActionManager& operator=(ActionManager&&) = delete;

      virtual ~ActionManager() = default;

      virtual void mapKey(Key key, ActionType actionType) = 0;

      virtual size_t addActionListener(ActionType aType, std::function<void(Action)> fn) = 0;
      virtual void removeActionListener(ActionType aType, size_t position) = 0;
   };
};
