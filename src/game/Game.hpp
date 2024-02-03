#pragma once

#include "core/Timer.hpp"

namespace Triton::Game {
   class Game {
    public:
      void update(const Core::Timer& timer);
      void render();
   };
}
