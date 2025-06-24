#pragma once

#include "UIState.hpp"

namespace tr {

using Timestamp = std::chrono::steady_clock::time_point;

class UIStateBuffer {
public:
  UIStateBuffer() = default;
  ~UIStateBuffer() = default;

  static constexpr size_t BufferSize = 9;

  struct Entry {
    Timestamp timestamp;
    UIState state;
    bool valid = false;
  };

  UIStateBuffer(const UIStateBuffer&) = delete;
  UIStateBuffer(UIStateBuffer&&) = delete;
  auto operator=(const UIStateBuffer&) -> UIStateBuffer& = delete;
  auto operator=(UIStateBuffer&&) -> UIStateBuffer& = delete;

  auto getStates(Timestamp t) -> std::optional<UIState>;
  auto pushState(const UIState& newState, Timestamp t) -> void;

private:
  std::array<Entry, BufferSize> buffer;
  std::atomic<size_t> writeIndex = 0;
  std::mutex bufferMutex;
};

}
