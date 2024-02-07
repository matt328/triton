#include "RenderSystem.hpp"

#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Resources.hpp"
#include "game/ecs/component/Transform.hpp"
#include "graphics/RenderObject.hpp"

namespace Triton::Game::Ecs::RenderSystem {
   void update(entt::registry& registry, Graphics::Renderer& renderer) {
      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderer.setCurrentCameraData(
          std::move(Graphics::CameraData{cam.view, cam.projection, cam.view * cam.projection}));

      const auto view = registry.view<Renderable, Transform>();

      for (auto [entity, renderable, transform] : view.each()) {
         auto renderObject =
             Graphics::RenderObject{renderable.meshId, renderable.textureId, transform.transform};
         renderer.enqueueRenderObject(std::move(renderObject));
      }
   }
}
