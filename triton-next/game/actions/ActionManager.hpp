#pragma once

#include "ActionType.hpp"
#include "ActionSet.hpp"
#include "../events/Key.hpp"

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

      [[nodiscard]] bool hasMapping(Key key) const;
      [[nodiscard]] ActionType mapKeyToAction(Key key) const;

      void setCurrentActionSet(const std::shared_ptr<ActionSet>& newCurrent);
      [[nodiscard]] std::shared_ptr<ActionSet> getCurrentActionSet() const;

    private:
      std::shared_ptr<ActionSet> currentActionSet;
   };

}
