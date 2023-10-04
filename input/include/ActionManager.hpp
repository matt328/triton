#pragma once

#include "Delegates.hpp"
#include "Key.hpp"
#include "ActionType.hpp"
#include "Vec2.hpp"

namespace Input {

   class ActionManager {
    public:
      ActionManager() = default;

      void mapKey(Key key, ActionType actionType);

      void keyPressed(Key key);
      void keyReleased(Key key);

      size_t onAction(ActionType aType, std::function<void(Action)> fn);
      void offAction(ActionType aType, size_t position);

    private:
      std::multimap<Key, ActionType> actionMap;
      std::unordered_map<ActionType, Delegate> delegatesMap;
   };

}