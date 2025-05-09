#include "ThreadedGameLoop.hpp"
#include "api/fx/IEventBus.hpp"
#include "api/fx/IGameWorldSystem.hpp"
#include "api/fx/IStateBuffer.hpp"
#include "api/gfx/SimState.hpp"
#include "gfx/IRenderContext.hpp"
#include "gfx/IWindow.hpp"

namespace tr {
ThreadedGameLoop::ThreadedGameLoop(std::shared_ptr<IEventBus> newEventBus,
                                   std::shared_ptr<IRenderContext> newRenderContext,
                                   std::shared_ptr<IWindow> newWindow,
                                   std::shared_ptr<IGuiSystem> newGuiSystem,
                                   std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                                   std::shared_ptr<TaskQueue> newTaskQueue,
                                   std::shared_ptr<IStateBuffer> newStateBuffer)
    : eventBus{std::move(newEventBus)},
      renderContext{std::move(newRenderContext)},
      window{std::move(newWindow)},
      guiSystem{std::move(newGuiSystem)},
      gameplaySystem{std::move(newGameWorldSystem)},
      taskQueue{std::move(newTaskQueue)},
      stateBuffer{std::move(newStateBuffer)} {

  Log.trace("Constructing FixedGameLoop");
  // TODO(matt): Move this functionality somewhere else?
  eventBus->subscribe<tr::WindowClosed>(
      [&]([[maybe_unused]] const tr::WindowClosed& event) { mainThreadRunning = false; });

  // Wire together game world to render world
  gameplaySystem->setRenderDataTransferHandler(
      [&](const RenderData& renderData) { renderContext->setRenderData(renderData); });

  Log.debug("Creating Default Camera");
  gameplaySystem->createDefaultCamera();
}

auto ThreadedGameLoop::run() -> void {
  gameplayThread = std::thread([&] { runGameplay(gameState.running, stateBuffer); });
  renderThread = std::thread([&] { runRenderer(rendererState.running, stateBuffer); });

  while (mainThreadRunning) {
    std::this_thread::sleep_for(std::chrono::microseconds(500));
  }

  gameState.running.store(false, std::memory_order_relaxed);
  rendererState.running.store(false, std::memory_order_relaxed);

  gameplayThread.join();
  renderThread.join();
}

auto ThreadedGameLoop::runGameplay(std::atomic<bool>& running,
                                   const std::shared_ptr<IStateBuffer>& stateBuffer) -> void {
  using Clock = std::chrono::steady_clock;
  auto t = Clock::now();
  auto dt = std::chrono::milliseconds(16); // 60Hz
  uint64_t frameCounter = 0;

  while (running) {
    auto now = Clock::now();
    if (now - t >= dt) {
      t += dt;

      auto* state = stateBuffer->getWriteSlot();
      gameplaySystem->fixedUpdate(/*state*/);
      stateBuffer->commitWrite();

    } else {
      std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
  }
}

auto ThreadedGameLoop::runRenderer(std::atomic<bool>& running,
                                   const std::shared_ptr<IStateBuffer>& stateBuffer) -> void {
  using Clock = std::chrono::steady_clock;
  auto currentTime = Clock::now();
  auto dt = std::chrono::milliseconds(16);
  auto accumulator = Clock::duration::zero();

  while (running) {
    auto newTime = Clock::now();
    auto frameTime = newTime - currentTime;
    if (frameTime > std::chrono::milliseconds(250)) {
      frameTime = std::chrono::milliseconds(250);
    }
    currentTime = newTime;
    accumulator += frameTime;

    window->pollEvents();

    float alpha = float(accumulator.count()) / dt.count();

    SimState prev{1024};
    SimState next{1024};
    if (stateBuffer->getInterpolatedStates(prev, next, alpha, currentTime)) {
      renderContext->setStates(prev, next, alpha);
    }

    renderContext->update();
    renderContext->renderNextFrame();

    FrameMark;
  }
}

}
