#pragma once

namespace tr {

class FrameState {
public:
  FrameState() = default;
  ~FrameState() = default;

  FrameState(const FrameState&) = delete;
  FrameState(FrameState&&) = delete;
  auto operator=(const FrameState&) -> FrameState& = delete;
  auto operator=(FrameState&&) -> FrameState& = delete;

  [[nodiscard]] auto getFrame() const {
    return currentFrame.load(std::memory_order_acquire);
  }

  auto advanceFrame() -> void {
    currentFrame.fetch_add(1, std::memory_order_release);
  }

  [[nodiscard]] auto getSwapchainImageIndex() const {
    return swapchainImageIndex.load(std::memory_order_acquire);
  }

  auto setSwapchainImageIndex(uint32_t index) {
    swapchainImageIndex.store(index, std::memory_order_relaxed);
  }

private:
  std::atomic<uint64_t> currentFrame = 0;
  std::atomic<uint32_t> swapchainImageIndex = 0;
};

}
