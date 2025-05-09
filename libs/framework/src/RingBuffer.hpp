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

  ~RingBuffer() override;

  explicit RingBuffer(const RingBufferConfig& config);

  auto getInterpolatedStates(SimState& stateA,
                             SimState& stateB,
                             float& alpha,
                             Timestamp currentTimeSec) -> bool override;

  auto getWriteSlot() -> SimState* override;
  auto commitWrite() -> void override;

private:
  size_t size;
  std::vector<SimState> buffer;
  std::atomic<size_t> writeIndex;
};
}
