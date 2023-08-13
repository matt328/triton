#include "TransformSystem.hpp"
#include "Transform.hpp"

TransformSystem::TransformSystem() = default;

void TransformSystem::update(entt::registry& registry, float dt) const {
   const auto view = registry.view<Transform>();

   for (auto [entity, transform] : view.each()) {
      float angle = dt * 5.f;
      transform.rotateByAxisAngle(glm::vec3(0.f, 1.f, 0.f), angle);
   }
}