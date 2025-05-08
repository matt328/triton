#include "RingBuffer.hpp"

namespace tr {

RingBuffer::RingBuffer(const RingBufferConfig& config)
    : size(config.capacity), readIndex(0), writeIndex(0) {
  assert(config.capacity > 1); // Capacity must be at least 2

  buffer.reserve(config.capacity);
  for (size_t i = 0; i < config.capacity; ++i) {
    buffer.emplace_back(config.maxObjectCount);
  }
}

auto RingBuffer::write(const SimState& state) -> bool {
  size_t currentWrite = writeIndex.load(std::memory_order_relaxed);
  size_t nextWrite = (currentWrite + 1) % size;

  if (nextWrite != readIndex.load(std::memory_order_acquire)) {
    buffer[currentWrite] = state;
    writeIndex.store(nextWrite, std::memory_order_release);
    return true;
  }
  return false;
}

auto RingBuffer::getInterpolatedStates(SimState& stateA,
                                       SimState& stateB,
                                       float& alpha,
                                       double currentTimeSec) -> bool {
  size_t size = buffer.size();
  size_t latest = (readIndex.load(std::memory_order_acquire) + size - 1) % size;
  size_t previous = (latest + size - 1) % size;

  if (latest == writeIndex || previous == writeIndex) {
    return false; // Not enough data
  }

  stateA = buffer[previous];
  stateB = buffer[latest];

  double tA = stateA.timeStamp;
  double tB = stateB.timeStamp;

  if (tB <= tA || currentTimeSec <= tA) {
    alpha = 0.0f;
  } else if (currentTimeSec >= tB) {
    alpha = 1.0f;
  } else {
    alpha = static_cast<float>((currentTimeSec - tA) / (tB - tA));
  }

  return true;
}

}
