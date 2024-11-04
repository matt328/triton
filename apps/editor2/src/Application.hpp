#pragma once

#include "cm/event/EventBus.hpp"

namespace tr::cm {
   class Timer;
   class IWindow;
}

namespace ed {

   class Properties;

   class Application {
    public:
      Application(std::shared_ptr<tr::cm::IWindow> window,
                  std::shared_ptr<Properties> properties,
                  tr::cm::evt::EventBus& eventBus,
                  std::shared_ptr<tr::cm::Timer> timer);
      ~Application();

      Application(const Application&) = default;
      Application(Application&&) = default;
      auto operator=(const Application&) -> Application& = delete;
      auto operator=(Application&&) -> Application& = delete;

      void run() const;

    private:
      std::shared_ptr<tr::cm::IWindow> window;
      std::shared_ptr<Properties> properties;
      std::shared_ptr<tr::cm::Timer> timer;
   };
}
