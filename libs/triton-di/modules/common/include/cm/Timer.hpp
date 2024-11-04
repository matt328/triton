#pragma once

namespace tr::gp {
   class IGameplaySystem;
}

namespace tr::gfx {
   class IRenderer;
}

namespace tr::cm {

   class IWindow;

   namespace evt {
      class EventBus;
   }

   class Timer {
    public:
      Timer(std::shared_ptr<IWindow> window,
            std::shared_ptr<gp::IGameplaySystem> gameplaySystem,
            std::shared_ptr<gfx::IRenderer> renderer,
            const std::shared_ptr<evt::EventBus>& newEventBus);
      ~Timer() = default;

      Timer(const Timer&) = default;
      Timer(Timer&&) = delete;
      auto operator=(const Timer&) -> Timer& = default;
      auto operator=(Timer&&) -> Timer& = delete;

      void start();

    private:
      bool paused{};
      bool running{true};
      std::shared_ptr<IWindow> window;
      std::shared_ptr<gp::IGameplaySystem> gameplaySystem;
      std::shared_ptr<gfx::IRenderer> renderer;
   };
}