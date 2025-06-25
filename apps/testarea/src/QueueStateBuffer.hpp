#pragma once

#include "bk/Chrono.h"

struct SimState {
  double value = 0.0;
  tr::Timestamp timeStamp;
};

class QueueStateBuffer {
public:
  QueueStateBuffer();
  ~QueueStateBuffer() = default;

  QueueStateBuffer(const QueueStateBuffer&) = delete;
  QueueStateBuffer(QueueStateBuffer&&) = delete;
  auto operator=(const QueueStateBuffer&) -> QueueStateBuffer& = delete;
  auto operator=(QueueStateBuffer&&) -> QueueStateBuffer& = delete;

  static constexpr size_t BufferSize = 6;

  auto getInterpolatedStates(SimState& stateA,
                             SimState& stateB,
                             float& alpha,
                             tr::Timestamp currentTimeSec) -> bool;

  auto getWriteSlot() -> SimState*;
  auto commitWrite() -> void;

  SimState buffer[BufferSize];

  std::atomic<int> writeIndex;
  int readIndex;
};
