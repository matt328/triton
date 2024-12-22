#include "DefaultContext.hpp"

#include "gp/Registry.hpp"
#include "tr/IGuiSystem.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "tr/IWindow.hpp"
#include "tr/IEventBus.hpp"

#include "gfx/IRenderContext.hpp"
#include "tr/IGameplaySystem.hpp"

namespace tr {

   static constexpr auto SleepMillis = 100;

   DefaultContext::DefaultContext(std::shared_ptr<IEventBus> newEventBus,
                                  std::shared_ptr<IRenderContext> newRenderContext,
                                  std::shared_ptr<tr::IWindow> newWindow,
                                  std::shared_ptr<IGuiSystem> newGuiSystem,
                                  std::shared_ptr<gp::Registry> newRegistry)
       : eventBus{std::move(newEventBus)},
         // gameplaySystem{std::move(newGameplaySystem)},
         renderContext{std::move(newRenderContext)},
         window{std::move(newWindow)},
         guiSystem{std::move(newGuiSystem)},
         registry{std::move(newRegistry)} {

      Log.trace("Constructing Default Context");

      eventBus->subscribe<tr::WindowClosed>(
          [&]([[maybe_unused]] const tr::WindowClosed& event) { running = false; });

      // Wire together game world to render world
      // gameplaySystem->setRenderDataTransferHandler(
      //     [&](const cm::gpu::RenderData& renderData) { renderContext->setRenderData(renderData);
      //     });
      //
      // gameplaySystem->createDefaultCamera();
   }

   void DefaultContext::run() {
      using Clock = std::chrono::steady_clock;
      using namespace std::literals;
      auto constexpr MaxFrameTime = 250ms;
      auto constexpr dt = std::chrono::duration<int64_t, std::ratio<1, 90>>{1};
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

         while (accumulator >= dt) {
            // gameplaySystem->fixedUpdate();
            t += dt;
            accumulator -= dt;
         }

         [[maybe_unused]] const auto alpha = accumulator / dt;

         {
            ZoneNamedN(z, "Gameplay Update", true);
            // gameplaySystem->update();
         }

         {
            ZoneNamedN(z, "RenderContext Render", true);
            renderContext->renderNextFrame();
         }
         FrameMark;
      }
      Log.debug("Main Loop Finished");
   }

   auto DefaultContext::getGameplaySystem() -> std::shared_ptr<gp::IGameplaySystem> {
      return nullptr;
   }

   auto DefaultContext::getGuiSystem() -> std::shared_ptr<IGuiSystem> {
      return guiSystem;
   }

   auto DefaultContext::getEventSystem() -> std::shared_ptr<IEventBus> {
      return eventBus;
   }

   auto DefaultContext::getRegistry() -> std::shared_ptr<gp::Registry> {
      return registry;
   }
}
