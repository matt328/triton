#pragma once

#include "Frame.hpp"

#include <vk/Swapchain.hpp>

namespace tr {
   class IFrameManager {
    public:
      IFrameManager() = default;
      virtual ~IFrameManager() = default;

      IFrameManager(const IFrameManager&) = delete;
      IFrameManager(IFrameManager&&) = delete;
      auto operator=(const IFrameManager&) -> IFrameManager& = delete;
      auto operator=(IFrameManager&&) -> IFrameManager& = delete;

      virtual auto acquireFrame()
          -> std::variant<std::reference_wrapper<Frame>, ImageAcquireResult> = 0;
   };
}
