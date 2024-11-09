#include "DefaultContext.hpp"

#include "gfx/IGraphicsDevice.hpp"
#include "tr/IWindow.hpp"
#include "tr/IEventBus.hpp"

#include "gfx/IRenderer.hpp"
#include "gp/IGameplaySystem.hpp"

namespace tr {

   constexpr auto SleepMillis = 100;

   DefaultContext::DefaultContext(std::shared_ptr<IEventBus> newEventBus,
                                  std::shared_ptr<gp::IGameplaySystem> newGameplaySystem,
                                  std::shared_ptr<gfx::IRenderer> newRenderer,
                                  std::shared_ptr<gfx::IGraphicsDevice> newGraphicsDevice,
                                  std::shared_ptr<tr::IWindow> newWindow)
       : eventBus{std::move(newEventBus)},
         gameplaySystem{std::move(newGameplaySystem)},
         renderer{std::move(newRenderer)},
         graphicsDevice{std::move(newGraphicsDevice)},
         window{std::move(newWindow)} {
      Log.trace("Constructing Default Context");

      eventBus->subscribe<tr::WindowClosed>(
          [&]([[maybe_unused]] const tr::WindowClosed& event) { running = false; });
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
            gameplaySystem->fixedUpdate();
            t += dt;
            accumulator -= dt;
         }

         [[maybe_unused]] const auto alpha = accumulator / dt;

         {
            ZoneNamedN(z, "Gameplay Update", true);
            gameplaySystem->update();
         }

         {
            ZoneNamedN(z, "RenderContext Render", true);
            renderer->render();
         }
         FrameMark;
      }
      Log.debug("Main Loop Finished");
   }
}
