#pragma once

#include "../events/Key.hpp"
#include "ActionType.hpp"

namespace Triton::Actions {
   class ActionSet {
    public:
      ActionSet() = default;
      ActionSet(const ActionSet&) = delete;
      ActionSet(ActionSet&&) = delete;
      ActionSet& operator=(const ActionSet&) = default;
      ActionSet& operator=(ActionSet&&) = delete;

      ~ActionSet() = default;

      void mapKey(Key key, ActionType actionType);
      [[nodiscard]] bool hasMapping(Key key) const;
      [[nodiscard]] ActionType mapKeyToAction(Key key) const;

    private:
      std::multimap<Key, ActionType> actionMap;
   };
};
