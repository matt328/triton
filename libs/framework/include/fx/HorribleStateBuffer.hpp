#pragma once

#include "api/fx/IStateBuffer.hpp"

namespace tr {

class HorribleStateBuffer : public IStateBuffer {
public:
  HorribleStateBuffer() = default;
  ~HorribleStateBuffer() override = default;

  static constexpr size_t BufferSize = 9;

  struct Entry {
    Timestamp timestamp;
    SimState state;
    bool valid = false;
  };

  HorribleStateBuffer(const HorribleStateBuffer&) = delete;
  HorribleStateBuffer(HorribleStateBuffer&&) = delete;
  auto operator=(const HorribleStateBuffer&) -> HorribleStateBuffer& = delete;
  auto operator=(HorribleStateBuffer&&) -> HorribleStateBuffer& = delete;

  auto getStates(Timestamp t) -> std::optional<std::pair<SimState, SimState>> override;
  auto pushState(const SimState& newState, Timestamp t) -> void override;

private:
  std::array<Entry, BufferSize> buffer;
  std::atomic<size_t> writeIndex = 0;
  std::mutex bufferMutex;
};

}
