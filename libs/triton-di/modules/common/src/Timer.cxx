#include "Timer.hpp"
#include "IWindow.hpp"
#include "event/EventBus.hpp"

#include "gp/IGameplaySystem.hpp"
#include "gfx/IRenderer.hpp"

namespace tr::cm {

   constexpr auto SleepMillis = 100;

   Timer::Timer(std::shared_ptr<IWindow> window,
                std::shared_ptr<gp::IGameplaySystem> gameplaySystem,
                std::shared_ptr<gfx::IRenderer> renderer,
                const std::shared_ptr<evt::EventBus>& newEventBus)
       : window{std::move(window)},
         gameplaySystem{std::move(gameplaySystem)},
         renderer{std::move(renderer)} {
      newEventBus->subscribe<evt::WindowClosed>(
          [&]([[maybe_unused]] const evt::WindowClosed& event) { running = false; });
   }

   void Timer::start() {

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