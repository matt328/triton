#pragma once

namespace tr {

class IGameLoop;
class IGuiSystem;
class IGraphicsContext;
class IGuiCallbackRegistrar;

class IFrameworkContext {
public:
  IFrameworkContext() = default;
  virtual ~IFrameworkContext() = default;

  IFrameworkContext(const IFrameworkContext&) = default;
  IFrameworkContext(IFrameworkContext&&) = delete;
  auto operator=(const IFrameworkContext&) -> IFrameworkContext& = default;
  auto operator=(IFrameworkContext&&) -> IFrameworkContext& = delete;

  virtual auto getGameLoop() -> std::shared_ptr<IGameLoop> = 0;
  virtual auto getGraphicsContext() -> std::shared_ptr<IGraphicsContext> = 0;
  virtual auto getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> = 0;
};
}
