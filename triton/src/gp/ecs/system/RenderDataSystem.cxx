#include "RenderDataSystem.hpp"

#include "gfx/ObjectData.hpp"
#include "gfx/RenderData.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/DebugConstants.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/Terrain.hpp"
#include "gp/ecs/component/Transform.hpp"

namespace tr::gp::ecs::RenderDataSystem {

   /// Loops through the registry and fills in the given RenderData struct
   void update(const entt::registry& registry, gfx::RenderData& renderData) {

      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderData.cameraData = gfx::CameraData{cam.view, cam.projection, cam.view * cam.projection};

      const auto view = registry.view<Renderable, Transform>();
      for (auto [entity, renderable, transform] : view.each()) {

         const auto isTerrainEntity = registry.any_of<TerrainMarker>(entity);

         for (auto& it : renderable.meshes) {
            const auto pos = renderData.objectData.size();
            if (isTerrainEntity) {
               renderData.terrainMeshData.emplace_back(it.first, pos);
            } else {
               renderData.staticMeshData.emplace_back(it.first, pos);
            }
            renderData.objectData.emplace_back(transform.transformation, it.second);
         }
      }

      const auto debugView = registry.view<Transform, DebugConstants>();
      for (const auto [entity, transform, debugConstants] : debugView.each()) {
         renderData.pushConstants.lightPosition = glm::vec4(transform.position, 1.f);
         renderData.pushConstants.params = glm::vec4(debugConstants.specularPower, 0.f, 0.f, 0.f);
      }
   }
}
