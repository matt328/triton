#pragma once

#include "api/gfx/SimState.hpp"

namespace tr {

class IStateBuffer {
public:
  IStateBuffer() = default;
  virtual ~IStateBuffer() = default;

  IStateBuffer(const IStateBuffer&) = default;
  IStateBuffer(IStateBuffer&&) = delete;
  auto operator=(const IStateBuffer&) -> IStateBuffer& = default;
  auto operator=(IStateBuffer&&) -> IStateBuffer& = delete;

  virtual auto write(const SimState& state) -> bool = 0;
  virtual auto getInterpolatedStates(SimState& stateA,
                                     SimState& stateB,
                                     float& alpha,
                                     Timestamp currentTime) -> bool = 0;

  virtual auto getWriteSlot() -> SimState* = 0;
  virtual auto commitWrite() -> void = 0;
};

}
