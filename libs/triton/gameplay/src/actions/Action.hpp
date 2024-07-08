#pragma once

#include "ActionType.hpp"

namespace tr::gp {

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
