#pragma once

#include "gp/ecs/component/Camera.hpp"
#include <entt/entity/fwd.hpp>
#include <shared_mutex>

namespace tr::gp {

   class EntitySystem {
    public:
      EntitySystem() = default;
      ~EntitySystem();

      EntitySystem(const EntitySystem&) = delete;
      EntitySystem& operator=(const EntitySystem&) = delete;

      EntitySystem(EntitySystem&&) = delete;
      EntitySystem& operator=(EntitySystem&&) = delete;

      [[nodiscard]] auto getCameraWriteLock() {
         return std::shared_lock<std::shared_mutex>{camerasMutex};
      }

      [[nodiscard]] auto& getRegistry() {
         return registry;
      }

      void writeCameras(std::function<void(entt::entity, ecs::Camera)> fn) {
         auto lock = std::shared_lock<std::shared_mutex>{camerasMutex};
         registry->view<ecs::Camera>().each(fn);
      }

    private:
      std::shared_mutex camerasMutex;
      std::unique_ptr<entt::registry> registry;
   };
}