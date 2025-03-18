#pragma once

namespace tr {

class IGameLoop {
public:
  IGameLoop() = default;
  virtual ~IGameLoop() = default;

  IGameLoop(const IGameLoop&) = default;
  IGameLoop(IGameLoop&&) = delete;
  auto operator=(const IGameLoop&) -> IGameLoop& = default;
  auto operator=(IGameLoop&&) -> IGameLoop& = delete;

  virtual auto run() -> void = 0;
};

}
