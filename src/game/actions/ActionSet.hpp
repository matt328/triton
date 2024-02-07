#pragma once

#include "../events/Key.hpp"
#include "../events/Mouse.hpp"
#include "ActionType.hpp"

namespace Triton::Actions {

   struct Source {
      Source(Events::MouseEvent me) : src(me) {
      }
      Source(Key key) : src(key) {
      }
      std::variant<Events::MouseEvent, Key> src;
   };

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
      [[nodiscard]] ActionType getActionForKey(Key key) const;
      [[nodiscard]] ActionType getActionForMouse(Events::MouseEvent mouse) const;

      void bindSource(Key key, ActionType actionType);
      void bindSource(Events::MouseEvent mouse, ActionType actionType);
      void bindSource(Source src, ActionType actionType);

    private:
      std::multimap<Key, ActionType> keyMap;
      std::multimap<Events::MouseEvent, ActionType> mouseMap;
   };
};
