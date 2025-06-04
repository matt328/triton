#pragma once

#include "api/fx/IStateBuffer.hpp"

namespace tr {

class DebugStateBuffer : public IStateBuffer {
public:
  DebugStateBuffer();
  ~DebugStateBuffer() override = default;

  DebugStateBuffer(const DebugStateBuffer&) = default;
  DebugStateBuffer(DebugStateBuffer&&) = delete;
  auto operator=(const DebugStateBuffer&) -> DebugStateBuffer& = default;
  auto operator=(DebugStateBuffer&&) -> DebugStateBuffer& = delete;

  auto getStates(uint64_t targetFrame) -> std::optional<std::pair<SimState, SimState>> override;
  auto pushState(const SimState& newState, uint64_t frameIndex) -> void override;

private:
  SimState previous;
  SimState next;
  float constAlpha{0.5f};
};

}
