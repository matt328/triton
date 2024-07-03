#pragma once

#include "gp/ecs/component/Camera.hpp"
#include <entt/entity/fwd.hpp>
#include <shared_mutex>

namespace tr::gp {

   class EntitySystem {
    public:
      EntitySystem();
      ~EntitySystem();

      EntitySystem(const EntitySystem&) = delete;
      EntitySystem& operator=(const EntitySystem&) = delete;

      EntitySystem(EntitySystem&&) = delete;
      EntitySystem& operator=(EntitySystem&&) = delete;

      [[nodiscard]] auto& getRegistry() {
         return registry;
      }

      void writeCameras(std::function<void(entt::entity, ecs::Camera)> fn);
      void writeCameras(
          std::function<void(entt::entity, ecs::Camera, uint32_t width, uint32_t height)> fn);

      void writeWindowDimensions(const std::pair<uint32_t, uint32_t> size);

    private:
      std::shared_mutex camerasMutex{};
      std::shared_mutex contextMutex{};
      std::unique_ptr<entt::registry> registry;
   };
}