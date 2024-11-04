#pragma once

namespace tr::cm::evt {
   enum class ActionType : uint32_t {
      MoveForward = 0,
      MoveBackward,
      StrafeLeft,
      StrafeRight,
      LookHorizontal,
      LookVertical,
      SelectionForward,
      SelectionBack,
      Ok,
      Cancel
   };

   enum class StateType : uint32_t {
      Action = 0,
      State,
      Range
   };

   struct Action {
      ActionType actionType{};
      StateType stateType{};
      std::variant<bool, float> value{};
   };
}