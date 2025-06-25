#pragma once

#include "api/gw/EditorState.hpp"
#include "bk/Chrono.h"

namespace tr {

class EditorStateBuffer {
public:
  EditorStateBuffer() = default;
  ~EditorStateBuffer() = default;

  EditorStateBuffer(const EditorStateBuffer&) = delete;
  EditorStateBuffer(EditorStateBuffer&&) = delete;
  auto operator=(const EditorStateBuffer&) -> EditorStateBuffer& = delete;
  auto operator=(EditorStateBuffer&&) -> EditorStateBuffer& = delete;

  static constexpr size_t BufferSize = 9;

  struct Entry {
    Timestamp timestamp;
    EditorState state;
    bool valid = false;
  };

  auto getStates(Timestamp t) -> std::optional<EditorState>;
  auto pushState(const EditorState& newState, Timestamp t) -> void;

private:
  std::array<Entry, BufferSize> buffer;
  std::atomic<size_t> writeIndex = 0;
  std::mutex bufferMutex;
};

}
