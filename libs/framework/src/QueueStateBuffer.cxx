#include "fx/QueueStateBuffer.hpp"

namespace tr {

QueueStateBuffer::QueueStateBuffer() : writeIndex{0}, readIndex{0} {
}

auto QueueStateBuffer::getInterpolatedStates(SimState& stateA,
                                             SimState& stateB,
                                             float& alpha,
                                             Timestamp currentTimeSec) -> bool {
  int localCount = count.load(std::memory_order_acquire);
  int localRead = readIndex;

  while (localCount >= 2) {
    const auto& a = buffer[localRead];
    const auto& b = buffer[(localRead + 1) % BufferSize];

    if (b.timeStamp < currentTimeSec) {
      readIndex = (localRead + 1) % BufferSize;
      count.fetch_sub(1, std::memory_order_release);
      localCount--;
      localRead = readIndex;
      continue;
    }

    if (a.timeStamp <= currentTimeSec && b.timeStamp >= currentTimeSec) {
      stateA = a;
      stateB = b;
      double duration = std::chrono::duration<double>(b.timeStamp - a.timeStamp).count();
      alpha = duration > 0.0
                  ? std::chrono::duration<double>(currentTimeSec - a.timeStamp).count() / duration
                  : 0.0f;

      readIndex = (localRead + 1) % BufferSize;
      count.fetch_sub(1, std::memory_order_release);
      return true;
    }

    break; // too far ahead, can't interpolate
  }

  return false;
}

auto QueueStateBuffer::getWriteSlot() -> SimState* {
  int localCount = count.load(std::memory_order_acquire);
  if (localCount >= BufferSize) {
    // Overwrite oldest entry by advancing readIndex
    readIndex = (readIndex + 1) % BufferSize;
    // count stays the same because we're overwriting
  } else {
    count.fetch_add(1, std::memory_order_release);
  }

  return &buffer[writeIndex];
}

auto QueueStateBuffer::commitWrite() -> void {
  writeIndex = (writeIndex + 1) % BufferSize;
  count.fetch_add(1, std::memory_order_release);
  std::cout << "commitWrite, timestamp="
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   buffer[writeIndex].timeStamp.time_since_epoch())
                   .count()
            << "\n";
}

}
