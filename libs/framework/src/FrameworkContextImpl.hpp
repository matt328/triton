#pragma once

#include "fx/IFrameworkContext.hpp"

namespace tr {

class FrameworkContextImpl : public IFrameworkContext {
public:
  FrameworkContextImpl(std::shared_ptr<IGameLoop> newGameLoop,
                       std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                       std::shared_ptr<ITaskQueue> newTaskQueue,
                       std::shared_ptr<IEventBus> newEventBus,
                       std::shared_ptr<IGameObjectProxy> newGameObjectProxy,
                       std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                       std::shared_ptr<IAssetService> newAssetService,
                       std::shared_ptr<IGameWorldContext> newGameWorldContext,
                       std::shared_ptr<IGraphicsContext> newGraphicsContext);
  ~FrameworkContextImpl() override = default;

  FrameworkContextImpl(const FrameworkContextImpl&) = default;
  FrameworkContextImpl(FrameworkContextImpl&&) = delete;
  auto operator=(const FrameworkContextImpl&) -> FrameworkContextImpl& = default;
  auto operator=(FrameworkContextImpl&&) -> FrameworkContextImpl& = delete;

  auto getGameLoop() -> std::shared_ptr<IGameLoop> override;
  auto getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> override;
  auto getTaskQueue() -> std::shared_ptr<ITaskQueue> override;
  auto getEventBus() -> std::shared_ptr<IEventBus> override;
  auto getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> override;
  auto getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> override;
  auto getAssetService() -> std::shared_ptr<IAssetService> override;

private:
  std::shared_ptr<IGameLoop> gameLoop;
  std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar;
  std::shared_ptr<ITaskQueue> taskQueue;
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<IGameObjectProxy> gameObjectProxy;
  std::shared_ptr<IGameWorldSystem> gameWorldSystem;
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<IGameWorldContext> gameWorldContext;
  std::shared_ptr<IGraphicsContext> graphicsContext;
};
}
