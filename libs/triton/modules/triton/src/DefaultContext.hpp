#pragma once

#include "tr/IGuiSystem.hpp"
#include "gfx/IRenderContext.hpp"
#include "tr/IContext.hpp"

namespace tr {

class IWindow;
class IEventBus;
class IGameplaySystem;

class DefaultContext final : public IContext {
public:
  explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus,
                          std::shared_ptr<IRenderContext> newRenderContext,
                          std::shared_ptr<IWindow> newWindow,
                          std::shared_ptr<IGuiSystem> newGuiSystem,
                          std::shared_ptr<IGameplaySystem> newGameplaySystem,
                          std::shared_ptr<TaskQueue> newTaskQueue);

  void run() override;
  auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> override;
  auto getGuiSystem() -> std::shared_ptr<IGuiSystem> override;
  auto getEventSystem() -> std::shared_ptr<IEventBus> override;
  auto getTaskQueue() -> std::shared_ptr<TaskQueue> override;

private:
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<IRenderContext> renderContext;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<IGuiSystem> guiSystem;
  std::shared_ptr<IGameplaySystem> gameplaySystem;
  std::shared_ptr<TaskQueue> taskQueue;

  bool paused{};
  bool running{true};
};
}
