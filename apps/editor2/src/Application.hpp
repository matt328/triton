#pragma once

#include "tr/IContext.hpp"
#include "tr/IGuiSystem.hpp"
#include "ui/Manager.hpp"

namespace ed {

class Properties;

class Application {
 public:
   Application(std::shared_ptr<Properties> newProperties,
               std::shared_ptr<tr::IContext> newContext,
               std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem,
               std::shared_ptr<tr::IGuiSystem> newGuiSystem,
               std::shared_ptr<Manager> newManager);
   ~Application();

   Application(const Application&) = default;
   Application(Application&&) = default;
   auto operator=(const Application&) -> Application& = delete;
   auto operator=(Application&&) -> Application& = delete;

   /// Runs the Application.
   void run() const;

 private:
   std::shared_ptr<Properties> properties;
   std::shared_ptr<tr::IContext> context;
   std::shared_ptr<tr::gp::IGameplaySystem> gameplaySystem;
   std::shared_ptr<tr::IGuiSystem> guiSystem;
   std::shared_ptr<Manager> manager;
};
}
