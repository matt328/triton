#pragma once

#include "gfx/RenderData.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gfx/Handles.hpp"
#include "gp/EntitySystemTypes.hpp"
#include <entt/entity/fwd.hpp>
#include "util/Timer.hpp"

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

      void fixedUpdate(const util::Timer& timer);
      void prepareRenderData(gfx::RenderData& renderData);

      auto createTerrain(const gfx::MeshHandles handles) -> gp::EntityType;

      void writeCameras(std::function<void(entt::entity, ecs::Camera)> fn);
      void writeCameras(
          std::function<void(entt::entity, ecs::Camera, uint32_t width, uint32_t height)> fn);

      void writeWindowDimensions(const std::pair<uint32_t, uint32_t> size);

    private:
      /*
        use a single mutex with shared/unique locks for simplicity.
        profile and watch for lock contention to identify areas where granularity needs to be
        increased
      */
      std::shared_mutex registryMutex{};
      std::unique_ptr<entt::registry> registry;
   };
}