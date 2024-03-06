#include "RenderSystem.hpp"

#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gfx/Renderer.hpp"
#include "gfx/RenderObject.hpp"

namespace tr::gp::ecs::RenderSystem {
   void update(entt::registry& registry,
               entt::delegate<void(gfx::RenderObject)>& renderObjectProducer,
               entt::delegate<void(gfx::CameraData)>& cameraDataProducer) {
      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();

      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      cameraDataProducer(gfx::CameraData{cam.view, cam.projection, cam.view * cam.projection});

      const auto view = registry.view<Renderable, Transform>();

      for (auto [entity, renderable, transform] : view.each()) {

         auto rotationMatrix = glm::rotate(glm::mat4(1.0f),
                                           glm::radians(transform.rotation.x),
                                           glm::vec3{1.f, 0.f, 0.f});
         rotationMatrix = glm::rotate(rotationMatrix,
                                      glm::radians(transform.rotation.y),
                                      glm::vec3{0.f, 1.f, 0.f});
         rotationMatrix = glm::rotate(rotationMatrix,
                                      glm::radians(transform.rotation.z),
                                      glm::vec3{0.f, 0.f, 1.f});

         auto translationMatrix = glm::translate(glm::mat4{1.f}, transform.position);

         auto transformMatrix = translationMatrix * rotationMatrix;

         auto renderObject =
             gfx::RenderObject{renderable.meshId, renderable.textureId, transformMatrix};
         renderObjectProducer(std::move(renderObject));
      }
   }
}