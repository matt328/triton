#pragma once

#include "Key.hpp"
#include "ActionType.hpp"
#include "Mouse.hpp"
#include "Gamepad.hpp"
#include "Sources.hpp"

/*
Source - abstraction over Key, Mouse(Button or Axis) and Gamepad(Button or Axis)
   - used by users to bind to ActionState
      actionSet.bind(Source{Key::Up}, ActionType::MoveForward);
      actionSet.bind(Source{GamePad::LeftStickY}, Action::MoveOnXAxis)
      actionSet.bind(Source{GamePad::RightStickY}, Action::LookVertical)
   - actionSet knows which sources are boolean and which are float.
ActionSet
   - holds mappings, the one marked current will translate Sources to ActionStates
ActionSystem
   - update()
      - clears the previous frame's ActionState
      - queries state of any Sources in the current actionSet
   - getActionState()
      - returns the ActionStates so it can be set as a registry context variable

*/

namespace Triton::Actions {

   class ActionSet {
    public:
      ActionSet() = default;
      ActionSet(const ActionSet&) = default;
      ActionSet(ActionSet&&) = delete;
      ActionSet& operator=(const ActionSet&) = default;
      ActionSet& operator=(ActionSet&&) = delete;

      ~ActionSet() = default;

      [[nodiscard]] const auto& getBoolMap() const {
         return boolMap;
      }

      [[nodiscard]] const auto& getFloatMap() const {
         return floatMap;
      }

      void mapBool(Source src, ActionType actionType);

      void mapFloat(Source src, ActionType actionType);

    private:
      std::unordered_multimap<ActionType, Source> boolMap;
      std::unordered_multimap<ActionType, Source> floatMap;
   };

}
