#pragma once

#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "fx/FrameworkConfig.hpp"

namespace tr {

class GameWorldContext;
class GraphicsContext;
class IEventQueue;
class IActionSystem;
class IStateBuffer;
class IApplication;
class IWindow;
class IAssetService;
class UIStateBuffer;

class ThreadedFrameworkContext {
public:
  static auto create(const FrameworkConfig& config,
                     std::shared_ptr<IGuiAdapter> guiAdapter,
                     std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar)
      -> std::shared_ptr<ThreadedFrameworkContext>;

  ThreadedFrameworkContext(const FrameworkConfig& config,
                           std::shared_ptr<IEventQueue> newEventQueue,
                           std::shared_ptr<IActionSystem> newActionSystem,
                           std::shared_ptr<IStateBuffer> newStateBuffer,
                           std::shared_ptr<IWindow> newWindow,
                           std::shared_ptr<IAssetService> newAssetService,
                           std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar);

  ~ThreadedFrameworkContext();

  ThreadedFrameworkContext(const ThreadedFrameworkContext&) = delete;
  ThreadedFrameworkContext(ThreadedFrameworkContext&&) = delete;
  auto operator=(const ThreadedFrameworkContext&) -> ThreadedFrameworkContext& = delete;
  auto operator=(ThreadedFrameworkContext&&) -> ThreadedFrameworkContext& = delete;

  auto startGameworld() -> void;
  auto startRenderer() -> void;

  auto runApplication(const std::shared_ptr<IApplication>& application) -> void;
  auto stop() -> void;

  auto getEventQueue() -> std::shared_ptr<IEventQueue>;

private:
  std::shared_ptr<IGuiAdapter> guiAdapter;
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IActionSystem> actionSystem;
  std::shared_ptr<IStateBuffer> stateBuffer;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar;
  std::shared_ptr<UIStateBuffer> uiStateBuffer;

  std::shared_ptr<GameWorldContext> gameWorldContext;
  std::shared_ptr<GraphicsContext> graphicsContext;

  std::jthread gameThread;
  std::jthread graphicsThread;

  auto configureGameWorldContext() -> std::shared_ptr<GameWorldContext>;
  auto configureGraphicsContext() -> std::shared_ptr<GraphicsContext>;
};

}
