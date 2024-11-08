#pragma once

#include "Sources.hpp"
#include "tr/Actions.hpp"

namespace tr::gp {
   class IActionSystem {
    public:
      explicit IActionSystem() = default;
      virtual ~IActionSystem() = default;

      virtual void mapSource(Source source, tr::StateType sType, tr::ActionType aType) = 0;
   };
}