#pragma once

namespace Triton::Actions {
   enum class ActionType : uint32_t {
      MoveForward = 0,
      MoveBackward,
      StrafeLeft,
      StrafeRight,
      SelectionForward,
      SelectionBack,
      Ok,
      Cancel
   };
}