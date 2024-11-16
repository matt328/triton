#pragma once

#include "tr/IContext.hpp"

namespace ed {

   class Properties;

   class Application {
    public:
      Application(std::shared_ptr<Properties> properties,
                  std::shared_ptr<tr::IContext> newContext,
                  std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem);
      ~Application();

      Application(const Application&) = default;
      Application(Application&&) = default;
      auto operator=(const Application&) -> Application& = delete;
      auto operator=(Application&&) -> Application& = delete;

      void run() const;

    private:
      std::shared_ptr<Properties> properties;
      std::shared_ptr<tr::IContext> context;
      std::shared_ptr<tr::gp::IGameplaySystem> gameplaySystem;
   };
}
