#pragma once

#include "api/fx/IStateBuffer.hpp"

namespace tr {

constexpr size_t BufferSize = 6;

class RenderProxyRingBuffer : public IStateBuffer {
public:
  struct Entry {
    uint64_t frameIndex;
    SimState state;
  };

  RenderProxyRingBuffer() = default;
  ~RenderProxyRingBuffer() override = default;

  RenderProxyRingBuffer(const RenderProxyRingBuffer&) = delete;
  RenderProxyRingBuffer(RenderProxyRingBuffer&&) = delete;
  auto operator=(const RenderProxyRingBuffer&) -> RenderProxyRingBuffer& = delete;
  auto operator=(RenderProxyRingBuffer&&) -> RenderProxyRingBuffer& = delete;

  auto pushState(const SimState& newState, uint64_t frameIndex) -> void override {
    const auto idx = writeIndex.fetch_add(1, std::memory_order_acq_rel) % BufferSize;
    buffer[idx].frameIndex = frameIndex;
    buffer[idx].state = newState;
  }

  auto getStates(uint64_t targetFrame) -> std::optional<std::pair<SimState, SimState>> override {

    std::array<Entry, BufferSize> snapshot;
    for (size_t i = 0; i < BufferSize; ++i) {
      snapshot[i] = buffer[i];
    }
    Entry* lower = nullptr;
    Entry* upper = nullptr;

    for (size_t i = 0; i < BufferSize; ++i) {
      const Entry& curr = snapshot[i];
      if (curr.frameIndex <= targetFrame) {
        if ((lower == nullptr) || curr.frameIndex > lower->frameIndex) {
          lower = const_cast<Entry*>(&curr);
        }
      }
      if (curr.frameIndex > targetFrame) {
        if (upper == nullptr || curr.frameIndex < upper->frameIndex) {
          upper = const_cast<Entry*>(&curr);
        }
      }
    }
    if (lower && upper && lower->frameIndex != upper->frameIndex) {
      return std::make_pair(lower->state, upper->state);
    }
    return std::nullopt;
  }

private:
  std::array<Entry, BufferSize> buffer;
  std::atomic<size_t> writeIndex = 0;
};

}
