#pragma once

namespace tr {

class IGameLoop;

class IFrameworkContext {
public:
  IFrameworkContext() = default;
  virtual ~IFrameworkContext() = default;

  IFrameworkContext(const IFrameworkContext&) = default;
  IFrameworkContext(IFrameworkContext&&) = delete;
  auto operator=(const IFrameworkContext&) -> IFrameworkContext& = default;
  auto operator=(IFrameworkContext&&) -> IFrameworkContext& = delete;

  virtual auto getGameLoop() -> std::shared_ptr<IGameLoop> = 0;
};
}
