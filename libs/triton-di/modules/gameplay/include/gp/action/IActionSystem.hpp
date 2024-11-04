#pragma once

#include "Sources.hpp"
#include "Action.hpp"

namespace tr::gp {
   class IActionSystem {
    public:
      IActionSystem() = default;
      ~IActionSystem() = default;

      virtual void mapSource(Source source, StateType sType, ActionType aType) = 0;
   };
}