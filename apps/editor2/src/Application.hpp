#pragma once

namespace tr::cm {
   class Timer;
}

namespace ed {

   class Properties;

   class Application {
    public:
      Application(std::shared_ptr<Properties> properties, std::shared_ptr<tr::cm::Timer> timer);
      ~Application();

      Application(const Application&) = default;
      Application(Application&&) = default;
      auto operator=(const Application&) -> Application& = delete;
      auto operator=(Application&&) -> Application& = delete;

      void run() const;

    private:
      std::shared_ptr<Properties> properties;
      std::shared_ptr<tr::cm::Timer> timer;
   };
}
