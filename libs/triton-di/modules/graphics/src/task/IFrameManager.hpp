#pragma once

#include "Frame.hpp"

namespace tr::gfx::task {
   class IFrameManager {
      virtual auto acquireFrame() -> Frame& = 0;
      virtual auto submitFrame(Frame& frame) -> void = 0;
   };
}
