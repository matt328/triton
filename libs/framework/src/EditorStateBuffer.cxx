#include "api/gw/EditorStateBuffer.hpp"

namespace tr {

auto EditorStateBuffer::getStates(Timestamp t) -> std::optional<EditorState> {
  std::array<Entry, BufferSize> snapshot;
  {
    std::lock_guard lock(bufferMutex);
    snapshot = buffer; // copy under mutex
  }

  Entry* closest = nullptr;

  for (auto& entry : snapshot) {
    if (!entry.valid || entry.timestamp > t) {
      continue;
    }
    if (closest == nullptr || entry.timestamp > closest->timestamp) {
      closest = &entry;
    }
  }

  if (closest != nullptr) {
    return closest->state;
  }

  return std::nullopt;
}

auto EditorStateBuffer::pushState(const EditorState& newState, Timestamp t) -> void {
  ZoneScoped;
  const size_t idx = writeIndex.fetch_add(1, std::memory_order_acq_rel) % BufferSize;
  std::lock_guard lock(bufferMutex);
  buffer[idx] = Entry{.timestamp = t, .state = newState, .valid = true};
}

}
