#pragma once

namespace tr {

class IGameLoop;
class IGuiSystem;
class IGraphicsContext;
class IGuiCallbackRegistrar;
class ITaskQueue;
class IEventBus;
class IEntityServiceProvider;
class IGameplaySystem;

class IFrameworkContext {
public:
  IFrameworkContext() = default;
  virtual ~IFrameworkContext() = default;

  IFrameworkContext(const IFrameworkContext&) = default;
  IFrameworkContext(IFrameworkContext&&) = delete;
  auto operator=(const IFrameworkContext&) -> IFrameworkContext& = default;
  auto operator=(IFrameworkContext&&) -> IFrameworkContext& = delete;

  virtual auto getGameLoop() -> std::shared_ptr<IGameLoop> = 0;
  virtual auto getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> = 0;
  virtual auto getTaskQueue() -> std::shared_ptr<ITaskQueue> = 0;
  virtual auto getEventBus() -> std::shared_ptr<IEventBus> = 0;
  virtual auto getEntityServiceProvider() -> std::shared_ptr<IEntityServiceProvider> = 0;
  virtual auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> = 0;
};
}
