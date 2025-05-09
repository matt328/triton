#pragma once

#include "QuitFlag.hpp"
#include "fx/IGameLoop.hpp"

namespace tr {

class IEventBus;
class IRenderContext;
class IWindow;
class IGuiSystem;
class IGameWorldSystem;
class TaskQueue;
class IStateBuffer;

static constexpr auto SleepMillis = 100;

class ThreadedGameLoop : public IGameLoop {
public:
  ThreadedGameLoop(std::shared_ptr<IEventBus> newEventBus,
                   std::shared_ptr<IRenderContext> newRenderContext,
                   std::shared_ptr<IWindow> newWindow,
                   std::shared_ptr<IGuiSystem> newGuiSystem,
                   std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                   std::shared_ptr<TaskQueue> newTaskQueue,
                   std::shared_ptr<IStateBuffer> newStateBuffer);
  ~ThreadedGameLoop() override = default;

  ThreadedGameLoop(const ThreadedGameLoop&) = delete;
  ThreadedGameLoop(ThreadedGameLoop&&) = delete;
  auto operator=(const ThreadedGameLoop&) -> ThreadedGameLoop& = delete;
  auto operator=(ThreadedGameLoop&&) -> ThreadedGameLoop& = delete;

  auto run() -> void override;

private:
  struct ThreadState {
    std::atomic<bool> paused{false};
    std::atomic<bool> running{true};
  };
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<IRenderContext> renderContext;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<IGuiSystem> guiSystem;
  std::shared_ptr<IGameWorldSystem> gameplaySystem;
  std::shared_ptr<TaskQueue> taskQueue;
  std::shared_ptr<IStateBuffer> stateBuffer;

  ThreadState gameState{};
  ThreadState rendererState{};

  std::thread gameplayThread;
  std::thread renderThread;

  bool mainThreadRunning = true;

  QuitFlag quitFlag{};

  auto runGameplay(std::atomic<bool>& running, const std::shared_ptr<IStateBuffer>& stateBuffer)
      -> void;
  auto runRenderer(std::atomic<bool>& running, const std::shared_ptr<IStateBuffer>& stateBuffer)
      -> void;
};

}
