#pragma once

#include "gfx/RenderData.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gfx/Handles.hpp"
#include "gp/EntitySystemTypes.hpp"
#include <entt/entity/fwd.hpp>
#include "util/Timer.hpp"

namespace tr::gfx::geo {
   class AnimationFactory;
}

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

      void fixedUpdate(const util::Timer& timer, gfx::geo::AnimationFactory& animationFactory);
      void prepareRenderData(gfx::RenderData& renderData);

      auto createTerrain(const gfx::MeshHandles handles) -> gp::EntityType;
      auto createStaticModel(const gfx::MeshHandles handles) -> gp::EntityType;
      auto createAnimatedModel(const gfx::LoadedSkinnedModelData modelData) -> gp::EntityType;
      auto createCamera(uint32_t width,
                        uint32_t height,
                        float fov,
                        float zNear,
                        float zFar,
                        glm::vec3 position,
                        std::optional<std::string> name) -> gp::EntityType;
      void setCurrentCamera(gp::EntityType);

      void removeAll();

      void writeCameras(std::function<void(entt::entity, ecs::Camera)> fn);
      void writeCameras(
          std::function<void(entt::entity, ecs::Camera, uint32_t width, uint32_t height)> fn);

      void writeWindowDimensions(const std::pair<uint32_t, uint32_t> size);

    private:
      std::shared_mutex registryMutex{};
      std::unique_ptr<entt::registry> registry;
   };
}