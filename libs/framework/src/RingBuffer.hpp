#pragma once

#include "api/fx/IStateBuffer.hpp"

namespace tr {

struct RingBufferConfig {
  size_t capacity;
  size_t maxObjectCount;
};

class RingBuffer : public IStateBuffer {
public:
  RingBuffer(const RingBuffer&) = delete;
  auto operator=(const RingBuffer&) -> RingBuffer& = delete;
  RingBuffer(RingBuffer&&) = delete;
  auto operator=(RingBuffer&&) -> RingBuffer& = delete;

  ~RingBuffer() override {
    buffer.clear();
  }

  explicit RingBuffer(const RingBufferConfig& config);

  /// Insert data at the write index
  /// @return false if the buffer is full and data was not inserted
  auto write(const SimState& state) -> bool override;

  auto getInterpolatedStates(SimState& stateA,
                             SimState& stateB,
                             float& alpha,
                             double currentTimeSec) -> bool override;

private:
  size_t size;
  std::vector<SimState> buffer;
  std::atomic<size_t> readIndex;
  std::atomic<size_t> writeIndex;
};
}
