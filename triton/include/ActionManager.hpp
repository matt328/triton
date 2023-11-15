#pragma once

#include "Action.hpp"
#include "ActionType.hpp"
#include "DefaultActionSet.hpp"
#include "Key.hpp"

namespace Triton::Actions {

   class ActionSet;

   class ActionManager {
    public:
      ActionManager() = default;
      ~ActionManager() = default;

      ActionManager(const ActionManager&) = default;
      ActionManager(ActionManager&&) = delete;
      ActionManager& operator=(const ActionManager&) = default;
      ActionManager& operator=(ActionManager&&) = delete;

      [[nodiscard]] std::unique_ptr<DefaultActionSet>& getCurrentActionSet();

      size_t createActionSet();
      void removeActionSet(size_t id);
      void setCurrentActionSet(size_t id);

      void keyPressed(Key key);
      void keyReleased(Key key);

    private:
      size_t currentActionSet{};
      std::vector<std::unique_ptr<DefaultActionSet>> actionSets;
   };

}
