#pragma once

#include "ActionManager.hpp"

namespace Triton::Actions {

   class DefaultActionManager : public ActionManager {
    public:
      void mapKey(Key key, ActionType actionType) override;

      size_t addActionListener(ActionType aType, std::function<void(Action)> fn) override;
      void removeActionListener(ActionType aType, size_t position) override;

      void keyPressed(Key key);
      void keyReleased(Key key);

    private:
      std::multimap<Key, ActionType> actionMap;
      std::unordered_map<ActionType, Delegate> delegatesMap;
   };

}