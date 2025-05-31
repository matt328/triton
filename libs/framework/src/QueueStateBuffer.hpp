#pragma once

#include "api/fx/IStateBuffer.hpp"

namespace tr {

class QueueStateBuffer : public IStateBuffer {
public:
  QueueStateBuffer();
  ~QueueStateBuffer() override = default;

  QueueStateBuffer(const QueueStateBuffer&) = delete;
  QueueStateBuffer(QueueStateBuffer&&) = delete;
  auto operator=(const QueueStateBuffer&) -> QueueStateBuffer& = delete;
  auto operator=(QueueStateBuffer&&) -> QueueStateBuffer& = delete;

  static constexpr size_t BufferSize = 6;

  auto getInterpolatedStates(SimState& stateA,
                             SimState& stateB,
                             float& alpha,
                             Timestamp currentTimeSec) -> bool override;

  auto getWriteSlot() -> SimState* override;
  auto commitWrite() -> void override;

private:
  std::array<SimState, BufferSize> buffer;
  size_t writeIndex;
  size_t readIndex;
  std::atomic<size_t> count;
};

}
