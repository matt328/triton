#pragma once

#include "gw/IEntityManager.hpp"

namespace tr {

class IEventQueue;
class FinalizerSystem;
class IStateBuffer;
class FrameState;

class EntityManager : public IEntityManager {
public:
  explicit EntityManager(std::shared_ptr<IEventQueue> newEventQueue,
                         std::shared_ptr<IStateBuffer> newStateBuffer);
  ~EntityManager() override;

  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  auto operator=(const EntityManager&) -> EntityManager& = delete;
  auto operator=(EntityManager&&) -> EntityManager& = delete;

  auto update() -> void override;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IStateBuffer> stateBuffer;

  std::unique_ptr<entt::registry> registry;
  std::unique_ptr<FinalizerSystem> finalizerSystem;
};

}
