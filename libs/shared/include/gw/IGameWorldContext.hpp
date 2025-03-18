#pragma once

namespace tr {

class IGameWorldSystem;

class IGameWorldContext {
public:
  IGameWorldContext() = default;
  virtual ~IGameWorldContext() = default;

  IGameWorldContext(const IGameWorldContext&) = default;
  IGameWorldContext(IGameWorldContext&&) = delete;
  auto operator=(const IGameWorldContext&) -> IGameWorldContext& = default;
  auto operator=(IGameWorldContext&&) -> IGameWorldContext& = delete;

  virtual auto getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> = 0;
};
}
