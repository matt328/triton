#pragma once

namespace tr {

class IGameLoop;
class IGuiSystem;
class IGraphicsContext;
class IGuiCallbackRegistrar;
class ITaskQueue;
class IEventBus;
class IGameObjectProxy;
class IGameWorldSystem;
class IAssetService;
class IGameWorldContext;
class IActionSystem;

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
  virtual auto getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> = 0;
  virtual auto getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> = 0;
  virtual auto getAssetService() -> std::shared_ptr<IAssetService> = 0;
  virtual auto getActionSystem() -> std::shared_ptr<IActionSystem> = 0;
};
}
