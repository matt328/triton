#pragma once

namespace tr::gp {
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
} // namespace tr::gp