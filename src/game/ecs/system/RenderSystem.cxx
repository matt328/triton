#include "RenderSystem.hpp"

#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Resources.hpp"
#include "game/ecs/component/Transform.hpp"

namespace Triton::Game::Ecs::RenderSystem {
   void update(entt::registry& registry, Graphics::Renderer& renderer) {
      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderer.setCurrentCameraData(
          std::move(Graphics::CameraData{cam.view, cam.projection, cam.view * cam.projection}));

      const auto view = registry.view<Renderable, Transform>();

      for (auto [entity, renderable, transform] : view.each()) {
         renderer.enqueueRenderObject(renderable.meshId, renderable.textureId, transform.transform);
      }
   }
}
