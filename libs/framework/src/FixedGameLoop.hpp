#pragma once

#include "fx/IGameLoop.hpp"

namespace tr {

class IEventBus;
class IRenderContext;
class IWindow;
class IGuiSystem;
class IGameWorldSystem;
class TaskQueue;

static constexpr auto SleepMillis = 100;

class FixedGameLoop : public IGameLoop {
public:
  FixedGameLoop(std::shared_ptr<IEventBus> newEventBus,
                std::shared_ptr<IRenderContext> newRenderContext,
                std::shared_ptr<IWindow> newWindow,
                std::shared_ptr<IGuiSystem> newGuiSystem,
                std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                std::shared_ptr<TaskQueue> newTaskQueue);
  ~FixedGameLoop() override = default;

  FixedGameLoop(const FixedGameLoop&) = default;
  FixedGameLoop(FixedGameLoop&&) = delete;
  auto operator=(const FixedGameLoop&) -> FixedGameLoop& = default;
  auto operator=(FixedGameLoop&&) -> FixedGameLoop& = delete;

  auto run() -> void override;

private:
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<IRenderContext> renderContext;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<IGuiSystem> guiSystem;
  std::shared_ptr<IGameWorldSystem> gameplaySystem;
  std::shared_ptr<TaskQueue> taskQueue;

  bool paused{};
  bool running{true};
};

}
