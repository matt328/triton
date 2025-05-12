#pragma once

namespace tr {

class IEventQueue;

class GameWorldContext {
public:
  explicit GameWorldContext(std::shared_ptr<IEventQueue> newEventQueue);
  ~GameWorldContext() = default;

  GameWorldContext(const GameWorldContext&) = default;
  GameWorldContext(GameWorldContext&&) = delete;
  auto operator=(const GameWorldContext&) -> GameWorldContext& = default;
  auto operator=(GameWorldContext&&) -> GameWorldContext& = delete;

  static auto create(std::shared_ptr<IEventQueue> newEventQueue)
      -> std::shared_ptr<GameWorldContext>;

  auto run() -> void;
  auto stop() -> void;

private:
  std::shared_ptr<IEventQueue> eventQueue;
};

}
