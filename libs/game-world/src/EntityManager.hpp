#pragma once

#include "api/fx/Events.hpp"
#include "gw/IEntityManager.hpp"

namespace tr {

class IEventQueue;
class FinalizerSystem;
class IStateBuffer;
class FrameState;
class CameraHandler;

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

  auto renderAreaCreated(const SwapchainCreated& event) -> void;
  auto renderAreaResized(const SwapchainResized& event) -> void;
  auto registerStaticModel(const StaticModelUploaded& event) -> void;
  auto createDefaultCamera() -> void;
};

}
