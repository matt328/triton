#pragma once

#include "api/gfx/SimState.hpp"

namespace tr {

class FinalizerSystem {
public:
  FinalizerSystem() = default;
  ~FinalizerSystem() = default;

  FinalizerSystem(const FinalizerSystem&) = default;
  FinalizerSystem(FinalizerSystem&&) = delete;
  auto operator=(const FinalizerSystem&) -> FinalizerSystem& = default;
  auto operator=(FinalizerSystem&&) -> FinalizerSystem& = delete;

  static auto update(entt::registry& registry, SimState& simState, Timestamp t) -> void;
};

}
