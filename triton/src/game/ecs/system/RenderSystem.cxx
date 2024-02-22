#include "RenderSystem.hpp"

#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Resources.hpp"
#include "game/ecs/component/Transform.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RenderObject.hpp"

namespace Triton::Game::Ecs::RenderSystem {
   void update(entt::registry& registry, Graphics::Renderer& renderer) {
      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderer.setCurrentCameraData(
          Graphics::CameraData{cam.view, cam.projection, cam.view * cam.projection});

      const auto view = registry.view<Renderable, Transform>();

      for (auto [entity, renderable, transform] : view.each()) {

         auto rotationMatrix =
             glm::rotate(glm::mat4(1.0f), glm::radians(transform.xRot), glm::vec3{1.f, 0.f, 0.f});
         rotationMatrix =
             glm::rotate(rotationMatrix, glm::radians(transform.yRot), glm::vec3{0.f, 1.f, 0.f});
         rotationMatrix =
             glm::rotate(rotationMatrix, glm::radians(transform.zRot), glm::vec3{0.f, 0.f, 1.f});

         auto translationMatrix =
             glm::translate(glm::mat4{1.f}, glm::vec3{transform.x, transform.y, transform.z});

         auto transformMatrix = translationMatrix * rotationMatrix;

         auto renderObject =
             Graphics::RenderObject{renderable.meshId, renderable.textureId, transformMatrix};
         renderer.enqueueRenderObject(std::move(renderObject));
      }
   }
}

namespace MM {
   template <>
   void ComponentEditorWidget<Triton::Game::Ecs::Transform>(entt::registry& reg,
                                                            entt::registry::entity_type e) {
      auto& t = reg.get<Triton::Game::Ecs::Transform>(e);
      // the "##Transform" ensures that you can use the name "x" in multiple labels
      ImGui::DragFloat("x##Transform", &t.x, 0.1f);
      ImGui::DragFloat("y##Transform", &t.y, 0.1f);
      ImGui::DragFloat("z##Transform", &t.z, 0.1f);

      ImGui::DragFloat("xRot##Transform", &t.xRot, 0.1f);
      ImGui::DragFloat("yRot##Transform", &t.yRot, 0.1f);
      ImGui::DragFloat("zRot##Transform", &t.zRot, 0.1f);
   }
}