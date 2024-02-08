#pragma once

#include "ActionSet.hpp"
#include "../events/Key.hpp"

namespace Triton::Actions {

   enum class ActionSets : uint32_t {
      Main = 0,
      Menu
   };

   class ActionSet;

   class ActionManager {
    public:
      ActionManager() = default;
      ~ActionManager() = default;

      ActionManager(const ActionManager&) = default;
      ActionManager(ActionManager&&) = delete;
      ActionManager& operator=(const ActionManager&) = default;
      ActionManager& operator=(ActionManager&&) = delete;

      ActionSet& createActionSet(ActionSets name);
      void setActiveSet(ActionSets newActiveSet);

      void keyPressed(Actions::Key key);
      void keyReleased(Actions::Key key);

    private:
      std::unordered_map<ActionSets, ActionSet> actionSetMap;
      std::unordered_map<
      ActionSets activeSet{};
   };

}
