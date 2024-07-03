#include "gp/EntitySystem.hpp"
#include <entt/entity/fwd.hpp>
#include "gp/ecs/component/Resources.hpp"

namespace tr::gp {
   EntitySystem::EntitySystem() {
      registry = std::make_unique<entt::registry>();
   }

   EntitySystem::~EntitySystem() {
   }

   void EntitySystem::writeCameras(std::function<void(entt::entity, ecs::Camera)> fn) {
      auto lock = std::unique_lock<std::shared_mutex>{camerasMutex};
      registry->view<ecs::Camera>().each(fn);
   }

   void EntitySystem::writeCameras(
       std::function<void(entt::entity, ecs::Camera, uint32_t width, uint32_t height)> fn) {

      auto camWriteLock = std::unique_lock<std::shared_mutex>{camerasMutex};
      auto contextLock = std::unique_lock<std::shared_mutex>{contextMutex};
      const auto [width, height] = registry->ctx().get<const ecs::WindowDimensions>();
      registry->view<ecs::Camera>().each(
          [&width, &height, &fn](auto entity, auto cam) { fn(entity, cam, width, height); });
   }

   void EntitySystem::writeWindowDimensions(const std::pair<uint32_t, uint32_t> size) {
      auto lock = std::shared_lock<std::shared_mutex>{contextMutex};
      registry->ctx().insert_or_assign<ecs::WindowDimensions>(
          ecs::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

}