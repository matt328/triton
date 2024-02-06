#include "RenderSystem.hpp"

#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Transform.hpp"

namespace Triton::Game::Ecs::RenderSystem {
   void update(entt::registry& registry, Graphics::Renderer& renderer) {
      const auto view = registry.view<Renderable, Transform>();
      for (auto [entity, renderable, transform] : view.each()) {
         renderer.enqueueRenderObject2(renderable.meshId,
                                       renderable.textureId,
                                       transform.transform);
      }
   }
}
