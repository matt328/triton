#pragma once

#include "vk/core/Swapchain.hpp"

namespace tr {

class Frame;

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
  [[nodiscard]] virtual auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>& = 0;
};

}
