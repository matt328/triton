#pragma once

#include "ActionType.hpp"
#include "ActionSet.hpp"
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

      [[nodiscard]] ActionSet& getCurrentActionSet() const;

      [[nodiscard]] bool hasMapping(Key key) const;
      [[nodiscard]] ActionType mapKeyToAction(Key key) const;

      size_t createActionSet();
      void removeActionSet(size_t id);
      void setCurrentActionSet(size_t id);

    private:
      size_t currentActionSet{};
      std::vector<std::shared_ptr<ActionSet>> actionSets;
   };

}
