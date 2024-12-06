#pragma once

#include "Frame.hpp"

namespace tr::gfx::task {
   class IFrameManager {
    public:
      IFrameManager() = default;
      virtual ~IFrameManager() = default;

      IFrameManager(const IFrameManager&) = delete;
      IFrameManager(IFrameManager&&) = delete;
      auto operator=(const IFrameManager&) -> IFrameManager& = delete;
      auto operator=(IFrameManager&&) -> IFrameManager& = delete;

      virtual auto acquireFrame() -> Frame& = 0;
      virtual auto submitFrame(Frame& frame) -> void = 0;
   };
}
