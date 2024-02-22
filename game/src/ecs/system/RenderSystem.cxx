#include "RenderSystem.hpp"

#include "ecs/component/Camera.hpp"
#include "ecs/component/Renderable.hpp"
#include "ecs/component/Resources.hpp"
#include "ecs/Transform.hpp"
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