#pragma once

#include "fx/IGameLoop.hpp"

namespace tr {

class FixedGameLoop : public IGameLoop {
public:
  FixedGameLoop() = default;
  ~FixedGameLoop() override = default;

  FixedGameLoop(const FixedGameLoop&) = default;
  FixedGameLoop(FixedGameLoop&&) = delete;
  auto operator=(const FixedGameLoop&) -> FixedGameLoop& = default;
  auto operator=(FixedGameLoop&&) -> FixedGameLoop& = delete;

  auto run() -> void override;
};

}
