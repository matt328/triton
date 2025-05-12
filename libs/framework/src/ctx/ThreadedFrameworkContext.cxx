#include "fx/ThreadedFrameworkContext.hpp"
#include "ActionSystem.hpp"
#include "EventQueue.hpp"
#include "GlfwWindow.hpp"
#include "RingBuffer.hpp"
#include "ThreadedGameLoop.hpp"
#include "bk/TaskQueue.hpp"
#include "fx/IGameLoop.hpp"

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
  const auto stateBuffer =
      std::make_shared<RingBuffer>(RingBufferConfig{.capacity = 6, .maxObjectCount = 1024});

  const auto window =
      std::make_shared<GlfwWindow>(WindowCreateInfo{.height = config.initialWindowSize.y,
                                                    .width = config.initialWindowSize.x,
                                                    .title = "Temporary Title"},
                                   eventQueue,
                                   guiAdapter);

  const auto frameworkInjector = di::make_injector(di::bind<IGameLoop>.to<ThreadedGameLoop>(),
                                                   di::bind<TaskQueue>.to<>(taskQueue),
                                                   di::bind<IEventQueue>.to<>(eventQueue),
                                                   di::bind<IActionSystem>.to<>(actionSystem),
                                                   di::bind<IStateBuffer>.to<>(stateBuffer),
                                                   di::bind<IWindow>.to<>(window));
  return frameworkInjector.create<std::shared_ptr<ThreadedFrameworkContext>>();
}

ThreadedFrameworkContext::ThreadedFrameworkContext(const FrameworkConfig& config,
                                                   std::shared_ptr<IEventQueue> newEventQueue,
                                                   std::shared_ptr<IActionSystem> newActionSystem,
                                                   std::shared_ptr<IStateBuffer> newStateBuffer)
    : eventQueue{std::move(newEventQueue)},
      actionSystem{std::move(newActionSystem)},
      stateBuffer{std::move(newStateBuffer)} {
}

auto ThreadedFrameworkContext::startGameworld() -> void {
  gameThread = std::thread([this] {
    gameWorldContext =
        createGameworldContext(const std::shared_ptr<IEventBus>& eventBus,
                               const std::shared_ptr<IAssetService>& assetService,
                               const std::shared_ptr<IActionSystem>& actionSystem,
                               const std::shared_ptr<IResourceProxy>& resourceProxy,
                               const std::shared_ptr<TaskQueue>& taskQueue,
                               const std::shared_ptr<GeometryGenerator>& geometryGenerator);
    gameWorldContext->run();
  });
}

auto ThreadedFrameworkContext::startRenderer() -> void {
  graphicsThread = std::thread([this] {
    graphicsContext =
        createVkGraphicsContext(VkGraphicsCreateInfo createInfo,
                                std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                                std::shared_ptr<IEventBus> newEventBus,
                                std::shared_ptr<TaskQueue> newTaskQueue,
                                std::shared_ptr<IStateBuffer> newStateBuffer,
                                std::shared_ptr<IWindow> newWindow);
    graphicsContext->run();
  });
}

auto ThreadedFrameworkContext::runMainLoop() -> void {
}

auto ThreadedFrameworkContext::stop() -> void {
  if (gameThread.joinable()) {
    gameThread.join();
  }
  if (graphicsThread.joinable()) {
    graphicsThread.join();
  }
}

}
