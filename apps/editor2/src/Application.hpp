#pragma once

#include "tr/IContext.hpp"

namespace ed {

   class Properties;

   class Application {
    public:
      Application(std::shared_ptr<Properties> properties,
                  std::shared_ptr<tr::IWindow> newWindow,
                  std::shared_ptr<tr::IContext> newContext);
      ~Application();

      Application(const Application&) = default;
      Application(Application&&) = default;
      auto operator=(const Application&) -> Application& = delete;
      auto operator=(Application&&) -> Application& = delete;

      void run() const;

    private:
      std::shared_ptr<Properties> properties;
      std::shared_ptr<tr::IWindow> window;
      std::shared_ptr<tr::IContext> context;
   };
}
