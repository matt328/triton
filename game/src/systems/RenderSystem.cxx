#include "RenderSystem.hpp"
#include "RenderObject.hpp"
#include "components/Renderable.hpp"
#include "components/Transform.hpp"

// Update the internal renderObjects list so the renderer can just query it.
using std::tuple;

void RenderSystem::update(entt::registry& registry) {
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

   for (const auto view = registry.view<Camera>(); const auto entity : view) {
      // For now we only support one camera
      currentCamera = view.get<Camera>(entity);
   }
}

[[nodiscard]] std::tuple<glm::mat4, glm::mat4, glm::mat4> RenderSystem::getCameraParams() const {
   return {currentCamera.viewMatrix,
           currentCamera.projectionMatrix,
           currentCamera.projectionMatrix * currentCamera.viewMatrix};
};
