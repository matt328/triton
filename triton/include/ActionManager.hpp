#pragma once

#include "ActionManagerProxy.hpp"

namespace Triton::Actions {

   class ActionManager {
    public:
      void mapKey(Key key, ActionType actionType);

      size_t addActionListener(ActionType aType, std::function<void(Action)> fn);
      void removeActionListener(ActionType aType, size_t position);

      void keyPressed(Key key);
      void keyReleased(Key key);

    private:
      std::multimap<Key, ActionType> actionMap;
      std::unordered_map<ActionType, Delegate> delegatesMap;
   };

}