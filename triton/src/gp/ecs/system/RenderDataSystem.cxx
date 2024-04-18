#include "RenderDataSystem.hpp"

#include "gfx/ObjectData.hpp"
#include "gfx/RenderData.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/Transform.hpp"

namespace tr::gp::ecs::RenderDataSystem {
   void update(entt::registry& registry, gfx::RenderData& renderData) {

      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderData.cameraData = gfx::CameraData{cam.view, cam.projection, cam.view * cam.projection};

      const auto view = registry.view<Renderable, Transform>();
      for (auto [entity, renderable, transform] : view.each()) {
         for (auto& it : renderable.meshes) {
            renderData.meshHandles.emplace_back(it.first);
            renderData.objectData.emplace_back(transform.transformation, it.second);
         }
      }
   }
}
