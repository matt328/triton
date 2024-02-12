#pragma once

namespace Triton::Actions {
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
}