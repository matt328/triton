#include "RenderSystem.hpp"
#include "Renderable.hpp"
#include "graphics/RenderDevice.hpp"
#include "Transform.hpp"

// Update the internal renderObjects list so the renderer can just query it.
void RenderSystem::update(entt::registry& registry, float dt) {
   renderObjects.clear();
   for (const auto view = registry.view<Renderable, Transform>(); const auto entity : view) {
      auto& renderable = view.get<Renderable>(entity);
      auto& model = view.get<Transform>(entity);
      auto renderObject = RenderObject{
          .meshId = renderable.getMeshId(),
          .textureId = renderable.getTextureId(),
          .modelMatrix = model.getTransform(),
      };
      renderObjects.push_back(renderObject);
   }
}
