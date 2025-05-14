#pragma once

#include "gw/IEntityManager.hpp"

namespace tr {

class IEventQueue;

class EntityManager : public IEntityManager {
public:
  explicit EntityManager(std::shared_ptr<IEventQueue> newEventQueue);
  ~EntityManager() override;

  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  auto operator=(const EntityManager&) -> EntityManager& = delete;
  auto operator=(EntityManager&&) -> EntityManager& = delete;

  auto update() -> void override;

private:
  std::shared_ptr<IEventQueue> eventQueue;

  std::unique_ptr<entt::registry> registry;
};

}
