#include "QueueStateBuffer.hpp"

namespace tr {

QueueStateBuffer::QueueStateBuffer() : writeIndex{0}, readIndex{0}, count{0} {
}

// This function is admittedly ai slop and I need to finish grokking what needs to happen here.
auto QueueStateBuffer::getInterpolatedStates(SimState& stateA,
                                             SimState& stateB,
                                             float& alpha,
                                             Timestamp currentTimeSec) -> bool {
  Log.trace("getInterpolatedStates, count={}", count);
  while (count.load(std::memory_order_acquire) >= 2) {
    const auto& a = buffer[readIndex];
    const auto& b = buffer[(readIndex + 1) % BufferSize];

    if (b.timeStamp < currentTimeSec) {
      // Both states are too old, discard the oldest one
      readIndex = (readIndex + 1) % BufferSize;
      count.fetch_sub(1, std::memory_order_release);
      continue;
    }

    if (a.timeStamp <= currentTimeSec && b.timeStamp >= currentTimeSec) {
      stateA = a;
      stateB = b;
      double duration = std::chrono::duration<double>(b.timeStamp - a.timeStamp).count();
      alpha = duration > 0.0
                  ? std::chrono::duration<double>(currentTimeSec - a.timeStamp).count() / duration
                  : 0.0f;
      return true;
    }

    // We're ahead of all buffered states — need to wait for more input.
    break;
  }
  return false;
}

auto QueueStateBuffer::getWriteSlot() -> SimState* {
  if (count.load(std::memory_order_acquire) >= BufferSize) {
    return nullptr; // Buffer full
  }
  return &buffer[writeIndex];
}

auto QueueStateBuffer::commitWrite() -> void {
  writeIndex = (writeIndex + 1) % BufferSize;
  count.fetch_add(1, std::memory_order_release);
}

}
