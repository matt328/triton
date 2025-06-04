#include "EntityManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/fx/IStateBuffer.hpp"
#include "components/Renderable.hpp"
#include "components/Transform.hpp"
#include "systems2/FinalizerSystem.hpp"

namespace tr {

EntityManager::EntityManager(std::shared_ptr<IEventQueue> newEventQueue,
                             std::shared_ptr<IStateBuffer> newStateBuffer)
    : eventQueue{std::move(newEventQueue)}, stateBuffer{std::move(newStateBuffer)} {
  registry = std::make_unique<entt::registry>();
  Log.trace("Created EntityManager");

  eventQueue->subscribe<StaticModelUploaded>([this](const StaticModelUploaded& event) {
    Log.trace("EntityManager handling StaticModelUploaded, geometryHandle={}",
              event.geometryHandle.id);
    auto entityId = registry->create();

    registry->emplace<Renderable>(entityId, std::vector<Handle<Geometry>>{event.geometryHandle});
    registry->emplace<Transform>(entityId);

    eventQueue->emit(StaticModelResponse{.batchId = event.batchId,
                                         .requestId = event.requestId,
                                         .entityName = event.entityName,
                                         .gameObjectId = entt::to_integral(entityId)});
  });

  finalizerSystem = std::make_unique<FinalizerSystem>();
}

EntityManager::~EntityManager() {
  Log.trace("Destroying EntityManager");
}

auto EntityManager::update() -> void {
  Timestamp currentTime = std::chrono::steady_clock::now();
  SimState writeState = SimState{1};
  tr::FinalizerSystem::update(*registry, writeState, currentTime);
  stateBuffer->pushState(writeState, currentTime);
}

}
