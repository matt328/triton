#pragma once

#include "cm/Handles.hpp"
#include "cm/RenderData.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "cm/Timer.hpp"

#include "components/Camera.hpp"
#include <entt/entity/fwd.hpp>

namespace tr::gp {

   class AnimationFactory;

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

      void fixedUpdate(const cm::Timer& timer, const AnimationFactory& animationFactory);
      void prepareRenderData(cm::gpu::RenderData& renderData);

      auto createTerrain(const cm::ModelData& handles) -> cm::EntityType;
      auto createStaticModel(const cm::ModelData& handles) -> cm::EntityType;
      auto createAnimatedModel(const cm::ModelData& modelData) -> cm::EntityType;
      auto createCamera(uint32_t width,
                        uint32_t height,
                        float fov,
                        float zNear,
                        float zFar,
                        glm::vec3 position,
                        const std::optional<std::string>& name) -> cm::EntityType;
      void setCurrentCamera(cm::EntityType);

      [[nodiscard]] auto createDebugAABB(const glm::vec3& min, const glm::vec3& max)
          -> cm::EntityType;
      [[nodiscard]] auto createDebugTriangle(const std::array<glm::vec3, 3> vertices) const
          -> cm::EntityType;
      [[nodiscard]] auto createDebugLine(const glm::vec3& start, const glm::vec3& end)
          -> cm::EntityType;
      [[nodiscard]] auto createDebugPoint(const glm::vec3& position) const -> cm::EntityType;

      void removeAll();

      void writeCameras(const std::function<void(entt::entity, cmp::Camera)>& fn);
      void writeCameras(
          std::function<void(entt::entity, cmp::Camera, uint32_t width, uint32_t height)> fn);

      void writeWindowDimensions(std::pair<uint32_t, uint32_t> size);

    private:
      std::shared_mutex registryMutex{};
      std::unique_ptr<entt::registry> registry;
   };
}