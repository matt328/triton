#pragma once

#include "api/gfx/SimState.hpp"

namespace tr {

using Timestamp = std::chrono::steady_clock::time_point;

class IStateBuffer {
public:
  IStateBuffer() = default;
  virtual ~IStateBuffer() = default;

  IStateBuffer(const IStateBuffer&) = default;
  IStateBuffer(IStateBuffer&&) = delete;
  auto operator=(const IStateBuffer&) -> IStateBuffer& = default;
  auto operator=(IStateBuffer&&) -> IStateBuffer& = delete;

  virtual auto getStates(Timestamp t) -> std::optional<std::pair<SimState, SimState>> = 0;
  virtual auto pushState(const SimState& newState, Timestamp t) -> void = 0;
};

}
