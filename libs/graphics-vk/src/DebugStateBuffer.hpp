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

  auto getInterpolatedStates(SimState& stateA,
                             SimState& stateB,
                             float& alpha,
                             Timestamp currentTimeSec) -> bool override;

  auto getWriteSlot() -> SimState* override;
  auto commitWrite() -> void override;

private:
  SimState previous;
  SimState next;
  float constAlpha{0.5f};
};

}
