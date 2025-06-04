#include "fx/ThreadedFrameworkContext.hpp"
#include "ActionSystem.hpp"
#include "DefaultAssetService.hpp"
#include "EventQueue.hpp"
#include "GlfwWindow.hpp"
#include "api/fx/IApplication.hpp"
#include "api/fx/IAssetService.hpp"
#include "bk/TaskQueue.hpp"
#include "fx/HorribleStateBuffer.hpp"
#include "gw/GameWorldContext.hpp"
#include "gfx/GraphicsContext.hpp"

#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 11
#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto ThreadedFrameworkContext::create(const FrameworkConfig& config,
                                      std::shared_ptr<IGuiAdapter> guiAdapter)
    -> std::shared_ptr<ThreadedFrameworkContext> {

  const auto eventQueue = std::make_shared<EventQueue>();
  const auto actionSystem = std::make_shared<ActionSystem>(eventQueue);

  const auto taskQueue = std::make_shared<TaskQueue>(TaskQueueConfig{.maxQueueSize = 1024});
  const auto stateBuffer = std::make_shared<HorribleStateBuffer>();

  const auto window =
      std::make_shared<GlfwWindow>(WindowCreateInfo{.height = config.initialWindowSize.y,
                                                    .width = config.initialWindowSize.x,
                                                    .title = "Temporary Title"},
                                   eventQueue,
                                   guiAdapter);

  const auto frameworkInjector =
      di::make_injector(di::bind<TaskQueue>.to<>(taskQueue),
                        di::bind<IEventQueue>.to<>(eventQueue),
                        di::bind<IActionSystem>.to<>(actionSystem),
                        di::bind<IStateBuffer>.to<>(stateBuffer),
                        di::bind<IWindow>.to<>(window),
                        di::bind<IAssetService>.to<DefaultAssetService>());
  return frameworkInjector.create<std::shared_ptr<ThreadedFrameworkContext>>();
}

ThreadedFrameworkContext::~ThreadedFrameworkContext() {
  Log.trace("Destroying ThreadedFrameworkContext");
}

ThreadedFrameworkContext::ThreadedFrameworkContext([[maybe_unused]] const FrameworkConfig& config,
                                                   std::shared_ptr<IEventQueue> newEventQueue,
                                                   std::shared_ptr<IActionSystem> newActionSystem,
                                                   std::shared_ptr<IStateBuffer> newStateBuffer,
                                                   std::shared_ptr<IWindow> newWindow,
                                                   std::shared_ptr<IAssetService> newAssetService)
    : eventQueue{std::move(newEventQueue)},
      actionSystem{std::move(newActionSystem)},
      stateBuffer{std::move(newStateBuffer)},
      window{std::move(newWindow)},
      assetService{std::move(newAssetService)} {
}

auto ThreadedFrameworkContext::startGameworld() -> void {
  gameThread = std::jthread([this](std::stop_token token) {
    pthread_setname_np(pthread_self(), "GameWorld");
    try {
      gameWorldContext = GameWorldContext::create(eventQueue, stateBuffer);
      gameWorldContext->run(token);
      Log.trace("nulling out gameWorldContext");
      gameWorldContext = nullptr;
    } catch (const std::exception& e) {
      Log.error("Exception in game thread: {}", e.what());
    } catch (...) { Log.error("Unknown exception in game thread"); }
  });
}

auto ThreadedFrameworkContext::startRenderer() -> void {
  graphicsThread = std::jthread([this](std::stop_token token) {
    pthread_setname_np(pthread_self(), "GraphicsContext");
    try {
      graphicsContext = GraphicsContext::create(eventQueue, stateBuffer, window, assetService);
      graphicsContext->run(token);
      Log.trace("Nulling out graphicsContext");
      graphicsContext = nullptr;
    } catch (const std::exception& e) {
      Log.error("Exception in render thread: {}", e.what());
    } catch (...) { Log.error("Unknown exception in render thread"); }
  });
}

auto ThreadedFrameworkContext::runApplication(const std::shared_ptr<IApplication>& application)
    -> void {
  application->onStart();
  while (!window->shouldClose()) {
    window->pollEvents();
    application->onUpdate();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  application->onShutdown();
}

auto ThreadedFrameworkContext::stop() -> void {
  Log.trace("ThreadedFrameworkContext::stop()");

  Log.trace("gameThread.request_stop()");
  gameThread.request_stop();
  gameThread.join();

  Log.trace("graphicsThread.request_stop()");
  graphicsThread.request_stop();
  graphicsThread.join();
}

auto ThreadedFrameworkContext::getEventQueue() -> std::shared_ptr<IEventQueue> {
  return eventQueue;
}

}
