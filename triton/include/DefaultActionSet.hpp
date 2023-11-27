#pragma once

#include "ActionSet.hpp"

namespace Triton::Actions {

   class DefaultActionSet : public ActionSet {
    public:
      void mapKey(Key key, ActionType actionType) override;
      bool hasMapping(Key key) override;
      [[nodiscard]] ActionType mapKeyToAction(Key key) const override;

    private:
      std::multimap<Key, ActionType> actionMap;
   };

}
