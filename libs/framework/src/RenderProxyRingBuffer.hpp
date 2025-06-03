#pragma once

namespace tr {

template <typename T, size_t N>
class RenderProxyRingBuffer {
public:
  static_assert(N >= 2, "Buffer must have at least 2 entries");

  struct Entry {
    uint64_t frameIndex;
    T state;
  };

  RenderProxyRingBuffer() = default;
  ~RenderProxyRingBuffer() = default;

  RenderProxyRingBuffer(const RenderProxyRingBuffer&) = delete;
  RenderProxyRingBuffer(RenderProxyRingBuffer&&) = delete;
  auto operator=(const RenderProxyRingBuffer&) -> RenderProxyRingBuffer& = delete;
  auto operator=(RenderProxyRingBuffer&&) -> RenderProxyRingBuffer& = delete;

  auto pushState(const T& newState, uint64_t frameIndex) -> void {
    const auto idx = writeIndex.fetch_add(1, std::memory_order_acq_rel) % N;
    buffer[idx].frameIndex = frameIndex;
    buffer[idx].state = newState;
  }

  auto getStates(uint64_t targetFrame) -> std::optional<std::pair<T, T>> {
    std::array<Entry, N> snapshot;
    for (size_t i = 0; i < N; ++i) {
      snapshot[i] = buffer[i];
    }
    Entry* a = nullptr;
    Entry* b = nullptr;

    for (size_t i = 0; i < N; ++i) {
      const Entry& curr = snapshot[i];
      if (curr.frameIndex <= targetFrame) {
        if ((a == nullptr) || curr.frameIndex > a->frameIndex) {
          a = const_cast<Entry*>(&curr);
        }
      }
      if (curr.frameIndex >= targetFrame) {
        if ((b == nullptr) || curr.frameIndex < b->frameIndex) {
          b = const_cast<Entry*>(&curr);
        }
      }
    }
    if (a && b && a->frameIndex != b->frameIndex) {
      return std::make_pair(a->state, b->state);
    }
    return std::nullopt;
  }

private:
  std::array<Entry, N> buffer;
  std::atomic<size_t> writeIndex = 0;
};

}
