#include "TransformSystem.hpp"

#include "cmp/Transform.hpp"

namespace tr::gp::ecs::TransformSystem {

   void update(entt::registry& registry) {
      const auto view = registry.view<Transform>();
      for (auto [entity, transform] : view.each()) {
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

         transform.transformation = translationMatrix * rotationMatrix;
      }
   }

}