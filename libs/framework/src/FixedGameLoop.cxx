#include "FixedGameLoop.hpp"
#include "fx/IGameWorldSystem.hpp"
#include "fx/ITaskQueue.hpp"
#include "gfx/IRenderContext.hpp"
#include "gfx/IWindow.hpp"

namespace tr {

FixedGameLoop::FixedGameLoop(std::shared_ptr<IEventBus> newEventBus,
                             std::shared_ptr<IRenderContext> newRenderContext,
                             std::shared_ptr<IWindow> newWindow,
                             std::shared_ptr<IGuiSystem> newGuiSystem,
                             std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                             std::shared_ptr<ITaskQueue> newTaskQueue)
    : eventBus{std::move(newEventBus)},
      renderContext{std::move(newRenderContext)},
      window{std::move(newWindow)},
      guiSystem{std::move(newGuiSystem)},
      gameplaySystem{std::move(newGameWorldSystem)},
      taskQueue{std::move(newTaskQueue)} {
}

auto FixedGameLoop::run() -> void {
  using Clock = std::chrono::steady_clock;
  using namespace std::literals;
  auto constexpr MaxFrameTime = 250ms;
  auto constexpr dt = std::chrono::duration<int64_t, std::ratio<1, 240>>{1};
  using duration = decltype(Clock::duration{} + dt);
  using time_point = std::chrono::time_point<Clock, duration>;

  time_point t{};

  time_point currentTime = Clock::now();
  duration accumulator = 0s;

  while (running) {
    time_point newTime = Clock::now();
    auto frameTime = newTime - currentTime;
    if (frameTime > MaxFrameTime) {
      frameTime = MaxFrameTime;
    }
    currentTime = newTime;

    accumulator += frameTime;

    {
      ZoneNamedN(poll, "Poll", true);
      window->pollEvents();
    }

    if (this->paused) {
      std::this_thread::sleep_for(std::chrono::milliseconds(SleepMillis));
      continue;
    }

    do {
      TracyPlot("dt", dt.count());
      gameplaySystem->fixedUpdate();
      t += dt;
      accumulator -= dt;
    } while (accumulator >= dt);

    taskQueue->processCompleteTasks();

    [[maybe_unused]] const auto alpha = accumulator / dt;

    // fixedUpdateLerp(alpha);? how to interpolate based on alpha here?

    {
      ZoneNamedN(z, "Gameplay Update", true);
      // Updates the RenderData and sets it in the RenderContext
      gameplaySystem->update();
    }

    {
      ZoneNamedN(z, "RenderContext Update", true);
      renderContext->update();
    }

    {
      ZoneNamedN(z, "RenderContext Render", true);
      renderContext->renderNextFrame();
    }
    FrameMark;
  }
  Log.debug("Main Loop Finished");
}

}
