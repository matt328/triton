#include "FrameworkContextImpl.hpp"

namespace tr {

FrameworkContextImpl::FrameworkContextImpl(
    std::shared_ptr<IGameLoop> newGameLoop,
    std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
    std::shared_ptr<TaskQueue> newTaskQueue,
    std::shared_ptr<IEventBus> newEventBus,
    std::shared_ptr<IGameObjectProxy> newGameObjectProxy,
    std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
    std::shared_ptr<IAssetService> newAssetService,
    std::shared_ptr<IGameWorldContext> newGameWorldContext,
    std::shared_ptr<IGraphicsContext> newGraphicsContext,
    std::shared_ptr<IActionSystem> newActionSystem)
    : gameLoop{std::move(newGameLoop)},
      guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)},
      taskQueue{std::move(newTaskQueue)},
      eventBus{std::move(newEventBus)},
      gameObjectProxy{std::move(newGameObjectProxy)},
      gameWorldSystem{std::move(newGameWorldSystem)},
      assetService{std::move(newAssetService)},
      gameWorldContext{std::move(newGameWorldContext)},
      graphicsContext{std::move(newGraphicsContext)},
      actionSystem{std::move(newActionSystem)} {
}

auto FrameworkContextImpl::getGameLoop() -> std::shared_ptr<IGameLoop> {
  return gameLoop;
}

auto FrameworkContextImpl::getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> {
  return guiCallbackRegistrar;
}

auto FrameworkContextImpl::getTaskQueue() -> std::shared_ptr<TaskQueue> {
  return taskQueue;
}

auto FrameworkContextImpl::getEventBus() -> std::shared_ptr<IEventBus> {
  return eventBus;
}

auto FrameworkContextImpl::getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> {
  return gameObjectProxy;
}

auto FrameworkContextImpl::getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> {
  return gameWorldSystem;
}

auto FrameworkContextImpl::getAssetService() -> std::shared_ptr<IAssetService> {
  return assetService;
}

auto FrameworkContextImpl::getActionSystem() -> std::shared_ptr<IActionSystem> {
  return actionSystem;
}

}
