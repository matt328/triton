#pragma once

namespace tr {
class IGuiCallbackRegistrar;
class IGameLoop;
}

namespace ed {

class Properties;
class Manager;

class Application {
public:
  Application(std::shared_ptr<Properties> newProperties, std::shared_ptr<Manager> newManager);
  ~Application();

  Application(const Application&) = default;
  Application(Application&&) = default;
  auto operator=(const Application&) -> Application& = delete;
  auto operator=(Application&&) -> Application& = delete;

  /// Runs the Application.
  void run() const;

private:
  std::shared_ptr<Properties> properties;
  std::shared_ptr<Manager> manager;
};
}
