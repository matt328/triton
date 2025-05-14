#pragma once

namespace tr {

class IEventQueue;
class IEntityManager;

class GameWorldContext {
public:
  GameWorldContext(std::shared_ptr<IEventQueue> newEventQueue,
                   std::shared_ptr<IEntityManager> newEntityManager);
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
  std::shared_ptr<IEntityManager> entityManager;

  bool running{};
};

}
