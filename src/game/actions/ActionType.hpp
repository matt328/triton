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

inline std::string getString(const Triton::Actions::ActionType& actionType) {
   switch (actionType) {
      case Triton::Actions::ActionType::MoveForward:
         return "MoveForward";
      case Triton::Actions::ActionType::MoveBackward:
         return "MoveBackward";
      case Triton::Actions::ActionType::StrafeLeft:
         return "StrafeLeft";
      case Triton::Actions::ActionType::StrafeRight:
         return "StrafeRight";
      default:
         return "Unknown";
   }
}
// This operator should work but it doesn't and I don't know why yet.
inline std::ostream& operator<<(std::ostream& os, const Triton::Actions::ActionType& actionType) {
   os << getString(actionType);
   return os;
}