#pragma once

#include "Sources.hpp"
#include "cm/event/Actions.hpp"
#include "cm/event/EventBus.hpp"

namespace tr::gp {
   class IActionSystem {
    public:
      explicit IActionSystem() = default;
      virtual ~IActionSystem() = default;

      virtual void mapSource(Source source,
                             cm::evt::StateType sType,
                             cm::evt::ActionType aType) = 0;
   };
}