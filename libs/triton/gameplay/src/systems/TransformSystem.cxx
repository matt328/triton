#include "TransformSystem.hpp"

#include "components/Transform.hpp"

namespace tr::gp::sys::TransformSystem {

   void update(entt::registry& registry) {
      for (const auto view = registry.view<cmp::Transform>();
           auto [entity, transform] : view.each()) {
         auto rotationMatrix = rotate(glm::mat4(1.0f),
                                           glm::radians(transform.rotation.x),
                                           glm::vec3{1.f, 0.f, 0.f});
         rotationMatrix = rotate(rotationMatrix,
                                      glm::radians(transform.rotation.y),
                                      glm::vec3{0.f, 1.f, 0.f});
         rotationMatrix = rotate(rotationMatrix,
                                      glm::radians(transform.rotation.z),
                                      glm::vec3{0.f, 0.f, 1.f});

         auto translationMatrix = translate(glm::mat4{1.f}, transform.position);

         transform.transformation = translationMatrix * rotationMatrix;
      }
   }

}