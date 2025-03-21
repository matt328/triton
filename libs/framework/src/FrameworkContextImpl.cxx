#include "FrameworkContextImpl.hpp"

namespace tr {

FrameworkContextImpl::FrameworkContextImpl(
    std::shared_ptr<IGameLoop> newGameLoop,
    std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
    std::shared_ptr<ITaskQueue> newTaskQueue,
    std::shared_ptr<IEventBus> newEventBus,
    std::shared_ptr<IEntityServiceProvider> newEntityServiceProvider,
    std::shared_ptr<IGameplaySystem> newGameplaySystem,
    std::shared_ptr<IAssetService> newAssetService)
    : gameLoop{std::move(newGameLoop)},
      guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)},
      taskQueue{std::move(newTaskQueue)},
      eventBus{std::move(newEventBus)},
      entityServiceProvider{std::move(newEntityServiceProvider)},
      gameplaySystem{std::move(newGameplaySystem)},
      assetService{std::move(newAssetService)} {
}

auto FrameworkContextImpl::getGameLoop() -> std::shared_ptr<IGameLoop> {
  return gameLoop;
}

auto FrameworkContextImpl::getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> {
  return guiCallbackRegistrar;
}

auto FrameworkContextImpl::getTaskQueue() -> std::shared_ptr<ITaskQueue> {
  return taskQueue;
}

auto FrameworkContextImpl::getEventBus() -> std::shared_ptr<IEventBus> {
  return eventBus;
}

auto FrameworkContextImpl::getEntityServiceProvider() -> std::shared_ptr<IEntityServiceProvider> {
  return entityServiceProvider;
}

auto FrameworkContextImpl::getGameplaySystem() -> std::shared_ptr<IGameplaySystem> {
  return gameplaySystem;
}

auto FrameworkContextImpl::getAssetService() -> std::shared_ptr<IAssetService> {
  return assetService;
}

}
