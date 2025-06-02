#include "QueueStateBuffer.hpp"

namespace tr {

QueueStateBuffer::QueueStateBuffer() : writeIndex{0}, readIndex{0} {
}

auto QueueStateBuffer::getInterpolatedStates(SimState& stateA,
                                             SimState& stateB,
                                             float& alpha,
                                             Timestamp currentTimeSec) -> bool {
  int localWrite = writeIndex.load(std::memory_order_acquire);

  // No data
  if (readIndex == localWrite) {
    return false;
  }

  int i = readIndex;
  while ((i + 1) % BufferSize != localWrite) {
    const auto& a = buffer[i];
    const auto& b = buffer[(i + 1) % BufferSize];

    if (b.timeStamp < currentTimeSec) {
      // Too old, advance readIndex
      readIndex = (i + 1) % BufferSize;
      i = readIndex;
      continue;
    }

    if (a.timeStamp <= currentTimeSec && b.timeStamp >= currentTimeSec) {
      // Interpolate
      stateA = a;
      stateB = b;
      double duration = std::chrono::duration<double>(b.timeStamp - a.timeStamp).count();
      alpha = duration > 0.0
                  ? std::chrono::duration<double>(currentTimeSec - a.timeStamp).count() / duration
                  : 0.0f;

      // Mark `a` as consumed
      readIndex = (i + 1) % BufferSize;
      return true;
    }

    // We're ahead of buffered data
    break;
  }

  return false;
}

auto QueueStateBuffer::getWriteSlot() -> SimState* {
  int nextWrite = (writeIndex.load(std::memory_order_relaxed) + 1) % BufferSize;
  if (nextWrite == readIndex) {
    return nullptr; // Buffer full, would overwrite unread data
  }
  return &buffer[writeIndex.load(std::memory_order_relaxed)];
}

auto QueueStateBuffer::commitWrite() -> void {
  int current = writeIndex.load(std::memory_order_relaxed);
  writeIndex.store((current + 1) % BufferSize, std::memory_order_release);
}

}
