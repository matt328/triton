#pragma once

namespace tr {

class IEventQueue;
class IEntityManager;
class IStateBuffer;
class EditorStateBuffer;

class GameWorldContext {
public:
  GameWorldContext(std::shared_ptr<IEventQueue> newEventQueue,
                   std::shared_ptr<IEntityManager> newEntityManager,
                   std::shared_ptr<IStateBuffer> newStateBuffer);
  ~GameWorldContext() = default;

  GameWorldContext(const GameWorldContext&) = default;
  GameWorldContext(GameWorldContext&&) = delete;
  auto operator=(const GameWorldContext&) -> GameWorldContext& = default;
  auto operator=(GameWorldContext&&) -> GameWorldContext& = delete;

  static auto create(std::shared_ptr<IEventQueue> newEventQueue,
                     std::shared_ptr<IStateBuffer> newStateBuffer,
                     std::shared_ptr<EditorStateBuffer> newEditorStateBuffer)
      -> std::shared_ptr<GameWorldContext>;

  auto run(std::stop_token token) -> void;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IEntityManager> entityManager;
  std::shared_ptr<IStateBuffer> stateBuffer;
};

}
