#pragma once

using Timestamp = std::chrono::steady_clock::time_point;

struct SimState {
  double value = 0.0;
  Timestamp timeStamp;
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
                             Timestamp currentTimeSec) -> bool;

  auto getWriteSlot() -> SimState*;
  auto commitWrite() -> void;

  SimState buffer[BufferSize];

  std::atomic<int> writeIndex;
  int readIndex;
};
