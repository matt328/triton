#pragma once

#include "Timer.hpp"

namespace Triton::Game {
   class Game {
    public:
      void update(const Core::Timer& timer);
      void render();
   };
}