#include "RingBuffer.hpp"

namespace tr {

RingBuffer::RingBuffer(const RingBufferConfig& config) : size(config.capacity), writeIndex(0) {
  assert(config.capacity > 1); // Capacity must be at least 2

  buffer.reserve(config.capacity);
  for (size_t i = 0; i < config.capacity; ++i) {
    buffer.emplace_back(config.maxObjectCount);
  }
}

auto RingBuffer::getInterpolatedStates(SimState& stateA,
                                       SimState& stateB,
                                       float& alpha,
                                       Timestamp currentTimeSec) -> bool {
  const size_t write = writeIndex.load(std::memory_order_acquire);

  int64_t i = static_cast<int64_t>(write) - 1;
  auto count = static_cast<int64_t>(buffer.size());

  SimState* newer = nullptr;
  SimState* older = nullptr;

  for (int64_t j = 0; j < count; ++j, --i) {
    auto index = static_cast<size_t>((i + count) % count);
    SimState& s = buffer[index];

    if (s.timeStamp <= currentTimeSec) {
      older = &s;
      size_t nextIndex = (index + 1) % count;
      newer = &buffer[nextIndex];
      break;
    }
  }

  if ((older == nullptr) || (newer == nullptr)) {
    return false;
  }

  stateA = *older;
  stateB = *newer;

  const auto tA = stateA.timeStamp;
  const auto tB = stateB.timeStamp;

  alpha = (tB == tA) ? 0.0f : static_cast<float>((currentTimeSec - tA) / (tB - tA));

  alpha = std::clamp(alpha, 0.0f, 1.0f);
  return true;
}

auto RingBuffer::getWriteSlot() -> SimState* {
  const auto currentWriteIndex = writeIndex.load(std::memory_order_relaxed);
  return &buffer[currentWriteIndex];
}

auto RingBuffer::commitWrite() -> void {
  const auto nextWriteIndex = (writeIndex.load(std::memory_order_relaxed) + 1) % size;
  writeIndex.store(nextWriteIndex, std::memory_order_release);
}

}
