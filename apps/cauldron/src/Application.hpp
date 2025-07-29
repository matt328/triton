#pragma once

#include "api/fx/IApplication.hpp"

namespace tr {
class IGuiCallbackRegistrar;
class IGameLoop;
class IEventQueue;
class IGuiCallbackRegistrar;
}

namespace ed {

class Preferences;
class Manager;

class Application : public tr::IApplication {
public:
  Application(std::shared_ptr<Preferences> newPreferences,
              std::shared_ptr<Manager> newManager,
              std::shared_ptr<tr::IEventQueue> newEventQueue,
              std::shared_ptr<tr::IGuiCallbackRegistrar> newGuiCallbackRegistrar);

  ~Application() override;

  Application(const Application&) = delete;
  Application(Application&&) = delete;
  auto operator=(const Application&) -> Application& = delete;
  auto operator=(Application&&) -> Application& = delete;

  auto onStart() -> void override;
  auto onUpdate() -> void override;
  auto onShutdown() -> void override;

private:
  std::shared_ptr<Preferences> preferences;
  std::shared_ptr<Manager> manager;
  std::shared_ptr<tr::IEventQueue> eventQueue;
  std::shared_ptr<tr::IGuiCallbackRegistrar> guiCallbackRegistrar;
};
}
