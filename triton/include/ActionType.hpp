#pragma once

namespace Triton::Actions {
   enum class ActionType : uint32_t {
      MoveForward = 0,
      MoveBackward,
      StrafeLeft,
      StrafeRight
   };
}
inline std::ostream& operator<<(std::ostream& os, const Triton::Actions::ActionType actionType) {
   switch (actionType) {
      case Triton::Actions::ActionType::MoveForward:
         os << "MoveForward";
         break;
      case Triton::Actions::ActionType::MoveBackward:
         os << "MoveBackward";
         break;
      case Triton::Actions::ActionType::StrafeLeft:
         os << "StrafeLeft";
         break;
      case Triton::Actions::ActionType::StrafeRight:
         os << "StrafeRight";
         break;
   }
   return os;
}