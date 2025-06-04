#include "fx/HorribleStateBuffer.hpp"

namespace tr {

auto HorribleStateBuffer::pushState(const SimState& newState, Timestamp t) -> void {
  const size_t idx = writeIndex.fetch_add(1, std::memory_order_acq_rel) % BufferSize;
  std::lock_guard lock(bufferMutex);
  buffer[idx] = Entry{.timestamp = t, .state = newState, .valid = true};
}

auto HorribleStateBuffer::getStates(Timestamp target)
    -> std::optional<std::pair<SimState, SimState>> {
  std::array<Entry, BufferSize> snapshot;
  {
    std::lock_guard lock(bufferMutex);
    snapshot = buffer; // copy under mutex
  }

  Entry* lower = nullptr;
  Entry* upper = nullptr;

  for (size_t i = 0; i < BufferSize; ++i) {
    const auto& entry = snapshot[i];
    if (!entry.valid) {
      continue;
    }
    if (entry.timestamp <= target) {
      if (!lower || entry.timestamp > lower->timestamp) {
        lower = const_cast<Entry*>(&entry);
      }
    }
    if (entry.timestamp >= target) {
      if (!upper || entry.timestamp < upper->timestamp) {
        upper = const_cast<Entry*>(&entry);
      }
    }
  }

  if (lower && upper && lower->timestamp <= target && upper->timestamp >= target &&
      lower->timestamp != upper->timestamp) {
    return std::make_pair(lower->state, upper->state);
  }

  return std::nullopt;
}

}
