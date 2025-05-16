#include "EntityManager.hpp"

namespace tr {

EntityManager::EntityManager(std::shared_ptr<IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  registry = std::make_unique<entt::registry>();
  Log.trace("Created EntityManager");
}

EntityManager::~EntityManager() {
  Log.trace("Destroying EntityManager");
}

auto EntityManager::update() -> void {
}

}
